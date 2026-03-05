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
#  include <config.h>
#endif

/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/

#include <complex.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/types.h>
#include <regex.h>
#include <ctype.h>
#include "irplib_sdp_spectrum.h"

#ifdef IRPLIB_USE_FITS_UPDATE_CHECKSUM
#  include <fitsio2.h>
#endif

/*-----------------------------------------------------------------------------
                                   Defines
 -----------------------------------------------------------------------------*/

/* Manually setup the following while this code remains outside CPL proper: */
#ifndef cpl_error_set_regex

cpl_error_code
cpl_error_set_regex_macro(const char *, cpl_error_code, int,
                          const regex_t *, const char *,
                          unsigned, const char *, ...) CPL_ATTR_PRINTF(7,8);

#define cpl_error_set_regex(code, regcode, preg, ...)           \
    cpl_error_set_regex_macro(cpl_func, code, regcode, preg,    \
                              __FILE__, __LINE__, __VA_ARGS__)

#endif /* cpl_error_set_regex */


#define KEY_ARCFILE             "ARCFILE"
#define KEY_ORIGFILE            "ORIGFILE"
#define KEY_RA                  "RA"
#define KEY_RA_COMMENT          "[deg] Spectroscopic target position (J2000)"
#define KEY_DEC                 "DEC"
#define KEY_DEC_COMMENT         "[deg] Spectroscopic target position (J2000)"
#define KEY_EXPTIME             "EXPTIME"
#define KEY_EXPTIME_COMMENT     "[s] Total integration time per pixel"
#define KEY_TEXPTIME            "TEXPTIME"
#define KEY_TEXPTIME_COMMENT    "[s] Total integration time of all exposures"
#define KEY_TIMESYS             "TIMESYS"
#define KEY_TIMESYS_COMMENT     "Time system used"
#define KEY_MJDOBS              "MJD-OBS"
#define KEY_MJDOBS_COMMENT      "[d] Start of observations (days)"
#define KEY_MJDEND              "MJD-END"
#define KEY_MJDEND_COMMENT      "[d] End of observations (days)"
#define KEY_PRODLVL             "PRODLVL"
#define KEY_PRODLVL_VALUE       2
#define KEY_PRODLVL_COMMENT     "Phase 3 product level: 1-raw, 2-science grade, 3-advanced"
#define KEY_PROCSOFT            "PROCSOFT"
#define KEY_PROCSOFT_COMMENT    "ESO pipeline version"
#define KEY_PRODCATG            "PRODCATG"
#define KEY_PRODCATG_COMMENT    "Data product category"
#define KEY_ORIGIN              "ORIGIN"
#define KEY_ORIGIN_VALUE        "ESO"
#define KEY_ORIGIN_COMMENT      "European Southern Observatory"
#define KEY_EXT_OBJ             "EXT_OBJ"
#define KEY_EXT_OBJ_COMMENT     "TRUE if extended"
#define KEY_DISPELEM            "DISPELEM"
#define KEY_DISPELEM_COMMENT    "Dispersive element name"
#define KEY_SPECSYS             "SPECSYS"
#define KEY_SPECSYS_VALUE       "TOPOCENT"
#define KEY_SPECSYS_COMMENT     "Reference frame for spectral coordinates"
#define KEY_PROG_ID             "PROG_ID"
#define KEY_PROG_ID_COMMENT     "ESO programme identification"
#define KEY_OBID                "OBID"
#define KEY_OBID_COMMENT        "Observation block ID"
#define KEY_M_EPOCH             "M_EPOCH"
#define KEY_M_EPOCH_COMMENT     "TRUE if resulting from multiple epochs"
#define KEY_OBSTECH             "OBSTECH"
#define KEY_OBSTECH_COMMENT     "Technique for observation"
#define KEY_FLUXCAL             "FLUXCAL"
#define KEY_FLUXCAL_COMMENT     "Type of flux calibration (ABSOLUTE or UNCALIBRATED)"
#define KEY_CONTNORM            "CONTNORM"
#define KEY_CONTNORM_COMMENT    "TRUE if normalised to the continuum"
#define KEY_WAVELMIN            "WAVELMIN"
#define KEY_WAVELMIN_COMMENT    "[nm] Minimum wavelength"
#define KEY_WAVELMAX            "WAVELMAX"
#define KEY_WAVELMAX_COMMENT    "[nm] Maximum wavelength"
#define KEY_SPEC_BIN            "SPEC_BIN"
#define KEY_SPEC_BIN_COMMENT    "[nm] Wavelength bin size"
#define KEY_TOT_FLUX            "TOT_FLUX"
#define KEY_TOT_FLUX_COMMENT    "TRUE if photometric conditions and all source flux is captured"
#define KEY_FLUXERR             "FLUXERR"
#define KEY_FLUXERR_VALUE       -2
#define KEY_FLUXERR_COMMENT     "Uncertainty in flux scale (%)"
#define KEY_REFERENC            "REFERENC"
#define KEY_REFERENC_VALUE      " "
#define KEY_REFERENC_COMMENT    "Reference publication"
#define KEY_SPEC_RES            "SPEC_RES"
#define KEY_SPEC_RES_COMMENT    "Reference spectral resolving power"
#define KEY_SPEC_ERR            "SPEC_ERR"
#define KEY_SPEC_ERR_COMMENT    "[nm] Statistical error in spectral coordinate"
#define KEY_SPEC_SYE            "SPEC_SYE"
#define KEY_SPEC_SYE_COMMENT    "[nm] Systematic error in spectral coordinate"
#define KEY_LAMNLIN             "LAMNLIN"
#define KEY_LAMNLIN_COMMENT     "Number of arc lines used for the wavel. solution"
#define KEY_LAMRMS              "LAMRMS"
#define KEY_LAMRMS_COMMENT      "[nm] RMS of the residuals of the wavel. solution"
#define KEY_GAIN                "GAIN"
#define KEY_GAIN_COMMENT        "Conversion factor (e-/ADU) electrons per data unit"
#define KEY_DETRON              "DETRON"
#define KEY_DETRON_COMMENT      "Readout noise per output (e-)"
#define KEY_EFFRON              "EFFRON"
#define KEY_EFFRON_COMMENT      "Median effective readout noise (e-)"
#define KEY_SNR                 "SNR"
#define KEY_SNR_COMMENT         "Median signal to noise ratio per order"
#define KEY_NCOMBINE            "NCOMBINE"
#define KEY_NCOMBINE_COMMENT    "No. of combined raw science data files"
#define KEY_PROV                "PROV"
#define KEY_PROV_COMMENT        "Originating raw science file"
#define KEY_ASSON               "ASSON"
#define KEY_ASSON_COMMENT       "Associated file name"
#define KEY_ASSOC               "ASSOC"
#define KEY_ASSOC_COMMENT       "Associated file category"
#define KEY_ASSOM               "ASSOM"
#define KEY_ASSOM_COMMENT       "Associated file md5sum"
#define KEY_VOCLASS             "VOCLASS"
#define KEY_VOCLASS_VALUE       "SPECTRUM V2.0"
#define KEY_VOCLASS_COMMENT     "VO Data Model"
#define KEY_VOPUB               "VOPUB"
#define KEY_VOPUB_VALUE         "ESO/SAF"
#define KEY_VOPUB_COMMENT       "VO Publishing Authority"
#define KEY_TITLE               "TITLE"
#define KEY_TITLE_COMMENT       "Dataset title"
#define KEY_OBJECT              "OBJECT"
#define KEY_OBJECT_COMMENT      "Target designation"
#define KEY_OBJECT_PHDU_COMMENT "Original target."
#define KEY_APERTURE            "APERTURE"
#define KEY_APERTURE_COMMENT    "[deg] Aperture diameter"
#define KEY_TELAPSE             "TELAPSE"
#define KEY_TELAPSE_COMMENT     "[s] Total elapsed time"
#define KEY_TMID                "TMID"
#define KEY_TMID_COMMENT        "[d] MJD mid exposure"
#define KEY_SPEC_VAL            "SPEC_VAL"
#define KEY_SPEC_VAL_COMMENT    "[nm] Mean wavelength"
#define KEY_SPEC_BW             "SPEC_BW"
#define KEY_SPEC_BW_COMMENT     "[nm] Bandpass width = Wmax - Wmin"
#define KEY_TDMIN(n)            "TDMIN"#n
#define KEY_TDMIN1_COMMENT      "Start in spectral coordinate"
#define KEY_TDMAX(n)            "TDMAX"#n
#define KEY_TDMAX1_COMMENT      "Stop in spectral coordinate"
#define KEY_TUTYP               "TUTYP"
#define KEY_TUTYP_COMMENT       "IVOA data model element for field "
#define KEY_TUCD                "TUCD"
#define KEY_TUCD_COMMENT        "UCD for field "
#define KEY_TCOMM               "TCOMM"
#define KEY_TCOMM_COMMENT       "Description for field "
#define KEY_NELEM               "NELEM"
#define KEY_NELEM_COMMENT       "Length of the data arrays"
#define KEY_EXTNAME             "EXTNAME"
#define KEY_EXTNAME_VALUE       "SPECTRUM"
#define KEY_EXTNAME_COMMENT     "Extension name"
#define KEY_INHERIT             "INHERIT"
#define KEY_INHERIT_VALUE       CPL_TRUE
#define KEY_INHERIT_COMMENT     "Primary header keywords are inherited"

/* A regular expression to select all keywords relevant to a spectrum class. */
#define ALL_KEYS_REGEXP \
  "^(" KEY_RA "|" \
  KEY_DEC "|" \
  KEY_EXPTIME "|" \
  KEY_TEXPTIME "|" \
  KEY_TIMESYS "|" \
  KEY_MJDOBS "|" \
  KEY_MJDEND "|" \
  KEY_PRODLVL "|" \
  KEY_PROCSOFT "|" \
  KEY_PRODCATG "|" \
  KEY_ORIGIN "|" \
  KEY_EXT_OBJ "|" \
  KEY_DISPELEM "|" \
  KEY_SPECSYS "|" \
  KEY_PROG_ID "|" \
  KEY_OBID "[0-9]+|" \
  KEY_M_EPOCH "|" \
  KEY_OBSTECH "|" \
  KEY_FLUXCAL "|" \
  KEY_CONTNORM "|" \
  KEY_WAVELMIN "|" \
  KEY_WAVELMAX "|" \
  KEY_SPEC_BIN "|" \
  KEY_TOT_FLUX "|" \
  KEY_FLUXERR "|" \
  KEY_REFERENC "|" \
  KEY_SPEC_RES "|" \
  KEY_SPEC_ERR "|" \
  KEY_SPEC_SYE "|" \
  KEY_LAMNLIN "|" \
  KEY_LAMRMS "|" \
  KEY_GAIN "|" \
  KEY_DETRON "|" \
  KEY_EFFRON "|" \
  KEY_SNR "|" \
  KEY_NCOMBINE "|" \
  KEY_PROV "[0-9]+|" \
  KEY_ASSON "[0-9]+|" \
  KEY_ASSOC "[0-9]+|" \
  KEY_ASSOM "[0-9]+|" \
  KEY_VOCLASS "|" \
  KEY_VOPUB "|" \
  KEY_TITLE "|" \
  KEY_OBJECT "|" \
  KEY_APERTURE "|" \
  KEY_TELAPSE "|" \
  KEY_TMID "|" \
  KEY_SPEC_VAL "|" \
  KEY_SPEC_BW "|" \
  KEY_TDMIN(1) "|" \
  KEY_TDMAX(1) "|" \
  KEY_TUTYP "[0-9]+|" \
  KEY_TUCD "[0-9]+|" \
  KEY_TCOMM "[0-9]+|" \
  KEY_NELEM "|" \
  KEY_EXTNAME "|" \
  KEY_INHERIT ")$"

/* A regular expression to select keywords from all explicit SDP spectrum
 * keywords that should land up in the primary HDU. */
#define PRIMARY_HDU_KEYS_REGEXP \
  "^(" KEY_RA "|" \
  KEY_DEC "|" \
  KEY_EXPTIME "|" \
  KEY_TEXPTIME "|" \
  KEY_TIMESYS "|" \
  KEY_MJDOBS "|" \
  KEY_MJDEND "|" \
  KEY_PRODLVL "|" \
  KEY_PROCSOFT "|" \
  KEY_PRODCATG "|" \
  KEY_ORIGIN "|" \
  KEY_EXT_OBJ "|" \
  KEY_DISPELEM "|" \
  KEY_SPECSYS "|" \
  KEY_PROG_ID "|" \
  KEY_OBID "[0-9]+|" \
  KEY_M_EPOCH "|" \
  KEY_OBSTECH "|" \
  KEY_FLUXCAL "|" \
  KEY_CONTNORM "|" \
  KEY_WAVELMIN "|" \
  KEY_WAVELMAX "|" \
  KEY_SPEC_BIN "|" \
  KEY_TOT_FLUX "|" \
  KEY_FLUXERR "|" \
  KEY_REFERENC "|" \
  KEY_SPEC_RES "|" \
  KEY_SPEC_ERR "|" \
  KEY_SPEC_SYE "|" \
  KEY_LAMNLIN "|" \
  KEY_LAMRMS "|" \
  KEY_GAIN "|" \
  KEY_DETRON "|" \
  KEY_EFFRON "|" \
  KEY_SNR "|" \
  KEY_NCOMBINE "|" \
  KEY_PROV "[0-9]+|" \
  KEY_ASSON "[0-9]+|" \
  KEY_ASSOC "[0-9]+|" \
  KEY_ASSOM "[0-9]+|" \
  KEY_OBJECT ")$"

/* A regular expression to select keywords from all explicit SDP spectrum
 * keywords that should land up in the extension HDU. */
#define EXTENSION_HDU_KEYS_REGEXP \
  "^(" KEY_RA "|" \
  KEY_DEC "|" \
  KEY_VOCLASS "|" \
  KEY_VOPUB "|" \
  KEY_TITLE "|" \
  KEY_OBJECT "|" \
  KEY_APERTURE "|" \
  KEY_TELAPSE "|" \
  KEY_TMID "|" \
  KEY_SPEC_VAL "|" \
  KEY_SPEC_BW "|" \
  KEY_TDMIN(1) "|" \
  KEY_TDMAX(1) "|" \
  KEY_TUTYP "[0-9]+|" \
  KEY_TUCD "[0-9]+|" \
  KEY_TCOMM "[0-9]+|" \
  KEY_NELEM "|" \
  KEY_EXTNAME "|" \
  KEY_INHERIT ")$"


#define GET_SET_METHODS(param, keyname, type, rettype, defaultval, comment) \
  rettype irplib_sdp_spectrum_get_##param(const irplib_sdp_spectrum *self) \
  { \
    cpl_ensure(self != NULL, CPL_ERROR_NULL_INPUT, defaultval); \
    assert(self->proplist != NULL); \
    if (cpl_propertylist_has(self->proplist, keyname)) { \
      return cpl_propertylist_get_##type(self->proplist, keyname); \
    } else { \
      return defaultval; \
    } \
  } \
  cpl_error_code irplib_sdp_spectrum_reset_##param(irplib_sdp_spectrum *self) \
  { \
    cpl_ensure_code(self != NULL, CPL_ERROR_NULL_INPUT); \
    assert(self->proplist != NULL); \
    (void) cpl_propertylist_erase(self->proplist, keyname); \
    return CPL_ERROR_NONE; \
  } \
  cpl_error_code irplib_sdp_spectrum_set_##param(irplib_sdp_spectrum *self, \
                                                 rettype value) \
  { \
    cpl_error_code error; \
    cpl_ensure_code(self != NULL, CPL_ERROR_NULL_INPUT); \
    assert(self->proplist != NULL); \
    if (cpl_propertylist_has(self->proplist, keyname)) { \
      error = cpl_propertylist_set_##type(self->proplist, keyname, value); \
    } else { \
      error = cpl_propertylist_append_##type(self->proplist, keyname, value); \
      if (! error) { \
        error = cpl_propertylist_set_comment(self->proplist, keyname, comment);\
        if (error) { \
          /* Delete entry if we could not set the comment to maintain a */ \
          /* consistent state. */ \
          cpl_errorstate prestate = cpl_errorstate_get(); \
          (void) cpl_propertylist_erase(self->proplist, keyname); \
          cpl_errorstate_set(prestate); \
        } \
      } \
    } \
    return error; \
  } \
  cpl_error_code irplib_sdp_spectrum_copy_##param(irplib_sdp_spectrum *self, \
                              const cpl_propertylist *plist, const char *name) \
  { \
    /* Note: check for plist or name equal NULL is done in the CPL calls. */ \
    cpl_ensure_code(self != NULL, CPL_ERROR_NULL_INPUT); \
    assert(self->proplist != NULL); \
    if (cpl_propertylist_has(plist, name)) { \
      cpl_errorstate prestate = cpl_errorstate_get(); \
      rettype value = cpl_propertylist_get_##type(plist, name); \
      if (cpl_errorstate_is_equal(prestate)) { \
        return irplib_sdp_spectrum_set_##param(self, value); \
      } else { \
        return cpl_error_set_message(cpl_func, cpl_error_get_code(), \
                  "Could not set '%s'. Likely the source '%s' keyword has a" \
                  " different format or type.", keyname, name); \
      } \
    } else { \
      return cpl_error_set_message(cpl_func, CPL_ERROR_DATA_NOT_FOUND, \
                  "Could not set '%s' since the '%s' keyword was not found.", \
                  keyname, name); \
    } \
  }

#define GET_SET_ARRAY_METHODS(param, keyname, type, rettype, defaultval, \
                              comment) \
  rettype irplib_sdp_spectrum_get_##param(const irplib_sdp_spectrum *self, \
                                          cpl_size index) \
  { \
    char *name; \
    cpl_ensure(self != NULL, CPL_ERROR_NULL_INPUT, defaultval); \
    assert(self->proplist != NULL); \
    name = cpl_sprintf("%s%"CPL_SIZE_FORMAT, keyname, index); \
    rettype result = defaultval; \
    if (cpl_propertylist_has(self->proplist, name)) { \
      result = cpl_propertylist_get_##type(self->proplist, name); \
    } \
    cpl_free(name); \
    return result; \
  } \
  cpl_error_code irplib_sdp_spectrum_reset_##param(irplib_sdp_spectrum *self, \
                                                   cpl_size index) \
  { \
    char *name; \
    cpl_ensure_code(self != NULL, CPL_ERROR_NULL_INPUT); \
    assert(self->proplist != NULL); \
    name = cpl_sprintf("%s%"CPL_SIZE_FORMAT, keyname, index); \
    (void) cpl_propertylist_erase(self->proplist, name); \
    cpl_free(name); \
    return CPL_ERROR_NONE; \
  } \
  cpl_error_code irplib_sdp_spectrum_set_##param(irplib_sdp_spectrum *self, \
                                                 cpl_size index, rettype value)\
  { \
    cpl_error_code error; \
    char *name; \
    cpl_ensure_code(self != NULL, CPL_ERROR_NULL_INPUT); \
    assert(self->proplist != NULL); \
    name = cpl_sprintf("%s%"CPL_SIZE_FORMAT, keyname, index); \
    if (cpl_propertylist_has(self->proplist, name)) { \
      error = cpl_propertylist_set_##type(self->proplist, name, value); \
    } else { \
      error = cpl_propertylist_append_##type(self->proplist, name, value); \
      if (! error) { \
        error = cpl_propertylist_set_comment(self->proplist, name, comment);\
        if (error) { \
          /* Delete entry if we could not set the comment to maintain a */ \
          /* consistent state. */ \
          cpl_errorstate prestate = cpl_errorstate_get(); \
          (void) cpl_propertylist_erase(self->proplist, name); \
          cpl_errorstate_set(prestate); \
        } \
      } \
    } \
    cpl_free(name); \
    return error; \
  } \
  cpl_error_code irplib_sdp_spectrum_copy_##param(\
                              irplib_sdp_spectrum *self, cpl_size index, \
                              const cpl_propertylist *plist, const char *name) \
  { \
    /* Note: check for plist or name equal NULL is done in the CPL calls. */ \
    cpl_ensure_code(self != NULL, CPL_ERROR_NULL_INPUT); \
    assert(self->proplist != NULL); \
    if (cpl_propertylist_has(plist, name)) { \
      cpl_errorstate prestate = cpl_errorstate_get(); \
      rettype value = cpl_propertylist_get_##type(plist, name); \
      if (cpl_errorstate_is_equal(prestate)) { \
        return irplib_sdp_spectrum_set_##param(self, index, value); \
      } else { \
        return cpl_error_set_message(cpl_func, cpl_error_get_code(), \
                  "Could not set '%s%"CPL_SIZE_FORMAT"'. Likely the source" \
                  " '%s' keyword has a different format or type.", \
                  keyname, index, name); \
      } \
    } else { \
      return cpl_error_set_message(cpl_func, CPL_ERROR_DATA_NOT_FOUND, \
                  "Could not set '%s%"CPL_SIZE_FORMAT"' since the '%s'" \
                  " keyword was not found.", keyname, index, name); \
    } \
  }


#define GET_SET_METHODS_TYPE_BOOL(param, keyname, comment) \
  GET_SET_METHODS(param, keyname, bool, cpl_boolean, CPL_FALSE, comment)

#define GET_SET_METHODS_TYPE_DOUBLE(param, keyname, comment) \
  GET_SET_METHODS(param, keyname, double, double, NAN, comment)

#define GET_SET_METHODS_TYPE_INT(param, keyname, comment) \
  GET_SET_METHODS(param, keyname, int, int, -1, comment)

#define GET_SET_METHODS_TYPE_STRING(param, keyname, comment) \
  GET_SET_METHODS(param, keyname, string, const char *, NULL, comment)

#define GET_SET_ARRAY_METHODS_TYPE_INT(param, keyname, comment) \
  GET_SET_ARRAY_METHODS(param, keyname, int, int, -1, comment)

#define GET_SET_ARRAY_METHODS_TYPE_STRING(param, keyname, comment) \
  GET_SET_ARRAY_METHODS(param, keyname, string, const char *, NULL, comment)


#define IRPLIB_TYPE_NELEM  CPL_TYPE_LONG_LONG | CPL_TYPE_UNSPECIFIED

/*----------------------------------------------------------------------------*/
/**
 * @defgroup irplib_sdp_spectrum    SDP 1D spectrum
 *
 * This module implements a Science Data Product (SDP) 1D spectrum object.
 * Various functions are provided to manipulate this kind of spectrum object.
 */
/*----------------------------------------------------------------------------*/

/**@{*/

/*-----------------------------------------------------------------------------
                                   Type definition
 -----------------------------------------------------------------------------*/

/**
 * @internal
 * @brief Definition of SDP 1D spectrum structure.
 */
struct _irplib_sdp_spectrum_ {
  /* Indicates the number of data points of the spectrum. */
  cpl_size nelem;

  /* Stores all the SDP keywords for the primary header and table extension. */
  cpl_propertylist *proplist;

  /* The table for the spectrum data points. */
  cpl_table *table;
};


/**
 * @internal
 * @brief A record structure containing information about a keyword.
 */
typedef struct _irplib_keyword_record_ {
  /* The name of the keyword. */
  const char *name;

  /* The keyword's default comment. */
  const char *comment;

  /* The keyword's type code. */
  cpl_type type;

  /* Is the keyword an array keyword or not (e.g. PROVi). */
  cpl_boolean is_array_key;

} irplib_keyword_record;

/*-----------------------------------------------------------------------------
                                   Internal function prototypes
 -----------------------------------------------------------------------------*/

static cpl_boolean
_irplib_property_equal(const cpl_property *a, const cpl_property *b);

static cpl_boolean
_irplib_array_equal(const cpl_array *a, const cpl_array *b, cpl_size n);

static cpl_boolean
_irplib_table_column_equal(const cpl_table *a, const cpl_table *b,
                           const char *name, cpl_boolean only_intersect);

static cpl_error_code
_irplib_sdp_spectrum_copy_column(irplib_sdp_spectrum *self, const char *to_name,
                                 const cpl_table* table, const char *from_name);

static cpl_size
_irplib_sdp_spectrum_count_keywords(const irplib_sdp_spectrum *self,
                                    const char *regexp);

static cpl_size
_irplib_sdp_spectrum_get_column_index(const irplib_sdp_spectrum *self,
                                      const char *name);

static const char *
_irplib_sdp_spectrum_get_column_keyword(const irplib_sdp_spectrum *self,
                                        const char *name, const char *keyword);

static cpl_error_code
_irplib_sdp_spectrum_set_column_keyword(irplib_sdp_spectrum *self,
                                        const char *name,
                                        const char *value,
                                        const char *keyword,
                                        const char *comment);

static void
_irplib_sdp_spectrum_erase_column_keywords(irplib_sdp_spectrum *self,
                                           const char *name);

static char * _irplib_make_regexp(const cpl_propertylist *plist,
                                  const char *extra);

#ifndef NDEBUG
static cpl_boolean _irplib_keyword_table_is_sorted(
                            const irplib_keyword_record *table, size_t entries);
#endif

static const irplib_keyword_record *
_irplib_sdp_spectrum_get_keyword_record(const char *name);

/*-----------------------------------------------------------------------------
                                   Function codes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
 * @brief Creates a new spectrum object.
 */
/*----------------------------------------------------------------------------*/
irplib_sdp_spectrum * irplib_sdp_spectrum_new(void)
{
  irplib_sdp_spectrum * obj = cpl_malloc(sizeof(irplib_sdp_spectrum));
  obj->nelem = 0;
  obj->proplist = cpl_propertylist_new();
  obj->table = cpl_table_new(1);
  return obj;
}

/*----------------------------------------------------------------------------*/
/**
 * @brief Duplicates an existing spectrum object.
 */
/*----------------------------------------------------------------------------*/
irplib_sdp_spectrum *
irplib_sdp_spectrum_duplicate(const irplib_sdp_spectrum *other)
{
  irplib_sdp_spectrum * obj;

  cpl_ensure(other != NULL, CPL_ERROR_NULL_INPUT, NULL);

  assert(other->proplist != NULL);
  assert(other->table != NULL);

  obj = cpl_malloc(sizeof(irplib_sdp_spectrum));
  obj->nelem = other->nelem;
  obj->proplist = cpl_propertylist_duplicate(other->proplist);
  obj->table = cpl_table_duplicate(other->table);
  return obj;
}

/*----------------------------------------------------------------------------*/
/**
 * @brief Destroys an existing spectrum object.
 */
/*----------------------------------------------------------------------------*/
void irplib_sdp_spectrum_delete(irplib_sdp_spectrum *self)
{
  if (self != NULL) {
    assert(self->proplist != NULL);
    assert(self->table != NULL);
    cpl_propertylist_delete(self->proplist);
    cpl_table_delete(self->table);
    cpl_free(self);
  }
}

/*----------------------------------------------------------------------------*/
/**
 * @internal
 * @brief Check if the types and values of two properties match.
 */
/*----------------------------------------------------------------------------*/
static cpl_boolean
_irplib_property_equal(const cpl_property *a, const cpl_property *b)
{
  int value_not_equal;
  cpl_type type;
  const char *sa, *sb;

  assert(a != NULL);
  assert(b != NULL);

  /* Check the types are the same. */
  type = cpl_property_get_type(a);
  if (cpl_property_get_type(b) != type) return CPL_FALSE;

  /* Check that the values are the same. */
  switch (type) {
  case CPL_TYPE_CHAR:
    value_not_equal = cpl_property_get_char(a) != cpl_property_get_char(b);
    break;
  case CPL_TYPE_BOOL:
    value_not_equal = cpl_property_get_bool(a) != cpl_property_get_bool(b);
    break;
  case CPL_TYPE_INT:
    value_not_equal = cpl_property_get_int(a) != cpl_property_get_int(b);
    break;
  case CPL_TYPE_LONG:
    value_not_equal = cpl_property_get_long(a) != cpl_property_get_long(b);
    break;
  case CPL_TYPE_LONG_LONG:
    value_not_equal =
      cpl_property_get_long_long(a) != cpl_property_get_long_long(b);
    break;
  case CPL_TYPE_FLOAT:
    value_not_equal = cpl_property_get_float(a) != cpl_property_get_float(b);
    break;
  case CPL_TYPE_DOUBLE:
    value_not_equal = cpl_property_get_double(a) != cpl_property_get_double(b);
    break;
  case CPL_TYPE_STRING:
    sa = cpl_property_get_string(a);
    sb = cpl_property_get_string(b);
    if (sa == NULL && sb == NULL) {
      value_not_equal = 0;
    } else if (sa != NULL && sb != NULL) {
      value_not_equal = strcmp(sa, sb) != 0;
    } else {
      return CPL_FALSE;
    }
    break;
#ifdef _Complex_I
  case CPL_TYPE_FLOAT_COMPLEX:
    value_not_equal =
      cpl_property_get_float_complex(a) != cpl_property_get_float_complex(b);
    break;
  case CPL_TYPE_DOUBLE_COMPLEX:
    value_not_equal =
      cpl_property_get_double_complex(a) != cpl_property_get_double_complex(b);
    break;
#endif
  default:
    cpl_error_set_message(cpl_func, CPL_ERROR_INVALID_TYPE,
                          "Unsupported data type found in property '%s'.",
                          cpl_property_get_name(a));
    return CPL_FALSE;
  }
  if (value_not_equal) return CPL_FALSE;

  /* If we got here then the type and value must be equal so return true. */
  return CPL_TRUE;
}


/*----------------------------------------------------------------------------*/
/**
 * @internal
 * @brief Check if two arrays have the same elements.
 * @param a The first array to check.
 * @param b The second array to check.
 * @param n The number of elements to check, from [0..n-1].
 * @return CPL_TRUE if the first n elements are identical and CPL_FALSE
 *      otherwise. If an error occurs then an error code is set. This can be
 *      checked with @c cpl_error_get_code.
 */
/*----------------------------------------------------------------------------*/
static cpl_boolean
_irplib_array_equal(const cpl_array *a, const cpl_array *b, cpl_size n)
{
  cpl_type type;

  assert(a != NULL);
  assert(b != NULL);
  assert(n <= cpl_array_get_size(a));
  assert(n <= cpl_array_get_size(b));

  type = cpl_array_get_type(a);
  if (type != cpl_array_get_type(b)) return CPL_FALSE;

  if (type == CPL_TYPE_STRING) {
    /* Handle strings: */
    cpl_size i;
    const char **stra = cpl_array_get_data_string_const(a);
    const char **strb = cpl_array_get_data_string_const(b);
    cpl_error_ensure(stra != NULL && strb != NULL, cpl_error_get_code(),
                     return CPL_FALSE, "Failed to get %s data for array.",
                     cpl_type_get_name(type));
    for (i = 0; i < n; ++i) {
      if (stra[i] == NULL && strb[i] == NULL) continue;
      if (stra[i] == NULL || strb[i] == NULL) return CPL_FALSE;
      if (strcmp(stra[i], strb[i]) != 0) return CPL_FALSE;
    }

  } else {
    /* Handle fundamental types: */
    cpl_size size, i;
    const void *va, *vb;

    switch (type) {
    case CPL_TYPE_INT:
      size = sizeof(int);
      va = cpl_array_get_data_int_const(a);
      vb = cpl_array_get_data_int_const(b);
      break;
    case CPL_TYPE_LONG_LONG:
      size = sizeof(long long);
      va = cpl_array_get_data_long_long_const(a);
      vb = cpl_array_get_data_long_long_const(b);
      break;
    case CPL_TYPE_FLOAT:
      size = sizeof(float);
      va = cpl_array_get_data_float_const(a);
      vb = cpl_array_get_data_float_const(b);
      break;
    case CPL_TYPE_DOUBLE:
      size = sizeof(double);
      va = cpl_array_get_data_double_const(a);
      vb = cpl_array_get_data_double_const(b);
      break;
#ifdef _Complex_I
    case CPL_TYPE_FLOAT_COMPLEX:
      size = sizeof(_Complex float);
      va = cpl_array_get_data_float_complex_const(a);
      vb = cpl_array_get_data_float_complex_const(b);
      break;
    case CPL_TYPE_DOUBLE_COMPLEX:
      size = sizeof(_Complex double);
      va = cpl_array_get_data_double_complex_const(a);
      vb = cpl_array_get_data_double_complex_const(b);
      break;
#endif
    default:
      cpl_error_set_message(cpl_func, CPL_ERROR_INVALID_TYPE,
                            "Unsupported data type.");
      return CPL_FALSE;
    }
    cpl_error_ensure(va != NULL && vb != NULL, cpl_error_get_code(),
                     return CPL_FALSE, "Failed to get %s data for array.",
                     cpl_type_get_name(type));
    for (i = 0; i < n; ++i) {
      int valid_a = cpl_array_is_valid(a, i);
      int valid_b = cpl_array_is_valid(b, i);
      if (! valid_a && ! valid_b) continue;
      if (! valid_a || ! valid_b) return CPL_FALSE;
      const void *vai = (const char *)va + (size * i);
      const void *vbi = (const char *)vb + (size * i);
      if (memcmp(vai, vbi, size) != 0) return CPL_FALSE;
    }
  }

  /* If we get here then the first n elements of the arrays are equal. */
  return TRUE;
}

/*----------------------------------------------------------------------------*/
/**
 * @internal
 * @brief Check if the named table column is identical in both tables.
 * @note If @c only_intersect is CPL_TRUE then only the overlapping part of data
 *      arrays is checked. In other words, assume n1 is the column depth (i.e.
 *      length of an array table item) for table @c a and n2 is the depth for
 *      the item in table @c b at the same location. Then only the first N data
 *      elements are checked for equality in the arrays, where N = min(n1, n2).
 * @note The column format strings are ignored since these only affect printing
 *      when using @c printf. The format information is anyway lost between
 *      saves and loads of table information.
 */
/*----------------------------------------------------------------------------*/
static cpl_boolean
_irplib_table_column_equal(const cpl_table *a, const cpl_table *b,
                           const char *name, cpl_boolean only_intersect)
{
  cpl_type type;
  cpl_size nrows, na, nb, i;
  const char *sa, *sb;

  assert(a != NULL);
  assert(b != NULL);

  nrows = cpl_table_get_nrow(a);
  if (only_intersect) {
    cpl_size nrows2 = cpl_table_get_nrow(b);
    if (nrows2 < nrows) nrows = nrows2;
  } else {
    if (cpl_table_get_nrow(b) != nrows) return CPL_FALSE;
  }

  /* Column types must be the same. */
  type = cpl_table_get_column_type(a, name);
  if (cpl_table_get_column_type(b, name) != type) return CPL_FALSE;

  /* Column dimensions must be the same. */
  na = cpl_table_get_column_dimensions(a, name);
  nb = cpl_table_get_column_dimensions(b, name);
  if (na != nb) return CPL_FALSE;

  /* Check that the column unit is the same. */
  sa = cpl_table_get_column_unit(a, name);
  sb = cpl_table_get_column_unit(b, name);
  cpl_error_ensure(sa != NULL && sb != NULL, cpl_error_get_code(),
                   return CPL_FALSE,
                   "Failed to get unit strings for column '%s'.", name);
  if (strcmp(sa, sb) != 0) return CPL_FALSE;

  /* Check that the values are the same. For arrays we check that the parts of
   * the arrays that overlap are at least the same. */
  if (type & CPL_TYPE_POINTER) {
    cpl_errorstate prestate;
    /* Handle array cells: */
    const cpl_array **va = cpl_table_get_data_array_const(a, name);
    const cpl_array **vb = cpl_table_get_data_array_const(b, name);
    cpl_error_ensure(va != NULL && vb != NULL,
                     cpl_error_get_code(), return CPL_FALSE,
                     "Failed to get %s data for column '%s'.",
                     cpl_type_get_name(type), name);
    if (only_intersect) {
      for (i = 0; i < nrows; ++i) {
        /* If both arrays are NULL then they are equal,
         * but not if only one is NULL. */
        if (va[i] == NULL && vb[i] == NULL) continue;
        if (va[i] == NULL || vb[i] == NULL) return CPL_FALSE;
        prestate = cpl_errorstate_get();
        cpl_size n1 = cpl_array_get_size(va[i]);
        cpl_size n2 = cpl_array_get_size(vb[i]);
        cpl_size n = n1 < n2 ? n1 : n2;
        if (! _irplib_array_equal(va[i], vb[i], n)) return CPL_FALSE;
        cpl_error_ensure(cpl_errorstate_is_equal(prestate),
                         cpl_error_get_code(), return CPL_FALSE,
                         "Failed when trying to match %s data for column '%s'.",
                         cpl_type_get_name(type), name);
      }
    } else {
      for (i = 0; i < nrows; ++i) {
        /* If both arrays are NULL then they are equal,
         * but not if only one is NULL. */
        if (va[i] == NULL && vb[i] == NULL) continue;
        if (va[i] == NULL || vb[i] == NULL) return CPL_FALSE;
        prestate = cpl_errorstate_get();
        cpl_size n = cpl_array_get_size(va[i]);
        if (n != cpl_array_get_size(vb[i])) return CPL_FALSE;
        if (! _irplib_array_equal(va[i], vb[i], n)) return CPL_FALSE;
        cpl_error_ensure(cpl_errorstate_is_equal(prestate),
                         cpl_error_get_code(), return CPL_FALSE,
                         "Failed when trying to match %s data for column '%s'.",
                         cpl_type_get_name(type), name);
      }
    }

  } else if (type == CPL_TYPE_STRING) {
    /* Handle strings: */
    const char **va = cpl_table_get_data_string_const(a, name);
    const char **vb = cpl_table_get_data_string_const(b, name);
    cpl_error_ensure(va != NULL && vb != NULL,
                     cpl_error_get_code(), return CPL_FALSE,
                     "Failed to get %s data for column '%s'.",
                     cpl_type_get_name(type), name);
    if (only_intersect) {
      for (i = 0; i < nrows; ++i) {
        if (va[i] == NULL && vb[i] == NULL) continue;
        if (va[i] == NULL || vb[i] == NULL) return CPL_FALSE;
        size_t n1 = strlen(va[i]);
        size_t n2 = strlen(vb[i]);
        size_t n = n1 < n2 ? n1 : n2;
        if (strncmp(va[i], vb[i], (cpl_size)n) != 0) return CPL_FALSE;
      }
    } else {
      for (i = 0; i < nrows; ++i) {
        if (va[i] == NULL && vb[i] == NULL) continue;
        if (va[i] == NULL || vb[i] == NULL) return CPL_FALSE;
        if (strcmp(va[i], vb[i]) != 0) return CPL_FALSE;
      }
    }

  } else {
    /* Handle fundamental types and strings: */
    cpl_size size;
    const void *va, *vb;

    switch (type) {
    case CPL_TYPE_INT:
      size = sizeof(int);
      va = cpl_table_get_data_int_const(a, name);
      vb = cpl_table_get_data_int_const(b, name);
      break;
    case CPL_TYPE_LONG_LONG:
      size = sizeof(long long);
      va = cpl_table_get_data_long_long_const(a, name);
      vb = cpl_table_get_data_long_long_const(b, name);
      break;
    case CPL_TYPE_FLOAT:
      size = sizeof(float);
      va = cpl_table_get_data_float_const(a, name);
      vb = cpl_table_get_data_float_const(b, name);
      break;
    case CPL_TYPE_DOUBLE:
      size = sizeof(double);
      va = cpl_table_get_data_double_const(a, name);
      vb = cpl_table_get_data_double_const(b, name);
      break;
#ifdef _Complex_I
    case CPL_TYPE_FLOAT_COMPLEX:
      size = sizeof(_Complex float);
      va = cpl_table_get_data_float_complex_const(a, name);
      vb = cpl_table_get_data_float_complex_const(b, name);
      break;
    case CPL_TYPE_DOUBLE_COMPLEX:
      size = sizeof(_Complex double);
      va = cpl_table_get_data_double_complex_const(a, name);
      vb = cpl_table_get_data_double_complex_const(b, name);
      break;
#endif
    default:
      cpl_error_set_message(cpl_func, CPL_ERROR_INVALID_TYPE,
                          "Unsupported data type found in column '%s'.", name);
      return CPL_FALSE;
    }
    cpl_error_ensure(va != NULL && vb != NULL,
                     cpl_error_get_code(), return CPL_FALSE,
                     "Failed to get %s data for column '%s'.",
                     cpl_type_get_name(type), name);
    for (i = 0; i < nrows; ++i) {
      int valid_a = cpl_table_is_valid(a, name, i);
      int valid_b = cpl_table_is_valid(b, name, i);
      if (! valid_a && ! valid_b) continue;
      if (! valid_a || ! valid_b) return CPL_FALSE;
      const void *vai = (const char *)va + (size * i);
      const void *vbi = (const char *)vb + (size * i);
      if (memcmp(vai, vbi, size) != 0) return CPL_FALSE;
    }
  }

  /* If we got here then the columns must be equal so return true. */
  return CPL_TRUE;
}

/*----------------------------------------------------------------------------*/
/**
 * @brief Compares two spectra if they are equal.
 * @param a The first spectrum to compare.
 * @param a The second spectrum to compare.
 * @param only_intersect Boolean flag indicating if only the intersection
 *      between two spectra should be checked.
 *
 * If @c only_intersect is set to CPL_FALSE then @c a and @c b are checked for
 * an exact match. i.e. number of keywords or table columns must be the same and
 * all values identical.
 * If set to CPL_TRUE, the comparison is more forgiving. In this case @c a is
 * allowed to have keywords or table columns that @c b does not have and visa
 * versa. However, any keywords/columns that exist in both spectrum objects must
 * have identical values.
 */
/*----------------------------------------------------------------------------*/
cpl_boolean irplib_sdp_spectrum_equal(const irplib_sdp_spectrum *a,
                                      const irplib_sdp_spectrum *b,
                                      cpl_boolean only_intersect)
{
  cpl_errorstate prestate;
  cpl_size na, i;
  cpl_boolean no_match = CPL_FALSE;
  cpl_array *names;
  const char *name;
  const cpl_property *pa, *pb;

  cpl_ensure(a != NULL && b != NULL, CPL_ERROR_NULL_INPUT, CPL_FALSE);

  assert(a->proplist != NULL);
  assert(a->table != NULL);
  assert(b->proplist != NULL);
  assert(b->table != NULL);

  na = cpl_propertylist_get_size(a->proplist);

  if (only_intersect) {
    /* Check that the values are the same if the keywords are in both property
     * lists. (Ignore comments) */
    for (i = 0; i < na; ++i) {
      pa = cpl_propertylist_get_const(a->proplist, i);
      cpl_error_ensure(pa != NULL, cpl_error_get_code(), return CPL_FALSE,
                  "Failed to get property structure %"CPL_SIZE_FORMAT".", i);
      name = cpl_property_get_name(pa);
      cpl_error_ensure(name != NULL, cpl_error_get_code(), return CPL_FALSE,
                  "Failed to get the name for property %"CPL_SIZE_FORMAT".", i);
      pb = cpl_propertylist_get_property_const(b->proplist, name);
      if (pb != NULL) {
        prestate = cpl_errorstate_get();
        if (! _irplib_property_equal(pa, pb)) return CPL_FALSE;
        if (! cpl_errorstate_is_equal(prestate)) return CPL_FALSE;
      }
    }

    /* Check that the columns with the same names in both tables are identical
     * for the parts of the data arrays that overlap. */
    prestate = cpl_errorstate_get();
    na = cpl_table_get_ncol(a->table);
    names = cpl_table_get_column_names(a->table);
    for (i = 0; i < na; ++i) {
      name = cpl_array_get_string(names, i);
      cpl_error_ensure(name != NULL, cpl_error_get_code(), break,
                  "Failed to get the name for column %"CPL_SIZE_FORMAT".", i);
      if (cpl_table_has_column(b->table, name)) {
        if (! _irplib_table_column_equal(a->table, b->table, name, CPL_TRUE)) {
          no_match = CPL_TRUE;
          break;
        }
      }
    }
    cpl_array_delete(names);
    /* Check that no errors occurred and all columns were processed. */
    if (no_match || ! cpl_errorstate_is_equal(prestate)) return CPL_FALSE;

  } else { /* not only_intersect */
    cpl_size nb;
    if (a->nelem != b->nelem) return CPL_FALSE;

    /* Check that the property lists are identical. (Ignore comments) */
    nb = cpl_propertylist_get_size(b->proplist);
    if (na != nb) return CPL_FALSE;
    for (i = 0; i < na; ++i) {
      pa = cpl_propertylist_get_const(a->proplist, i);
      cpl_error_ensure(pa != NULL, cpl_error_get_code(), return CPL_FALSE,
                  "Failed to get property structure %"CPL_SIZE_FORMAT".", i);
      name = cpl_property_get_name(pa);
      cpl_error_ensure(name != NULL, cpl_error_get_code(), return CPL_FALSE,
                  "Failed to get the name for property %"CPL_SIZE_FORMAT".", i);
      pb = cpl_propertylist_get_property_const(b->proplist, name);
      if (pb == NULL) return CPL_FALSE;
      prestate = cpl_errorstate_get();
      if (! _irplib_property_equal(pa, pb)) return CPL_FALSE;
      if (! cpl_errorstate_is_equal(prestate)) return CPL_FALSE;
    }

    /* Check that the tables are identical. */
    prestate = cpl_errorstate_get();
    na = cpl_table_get_ncol(a->table);
    nb = cpl_table_get_ncol(b->table);
    if (na != nb) return CPL_FALSE;
    names = cpl_table_get_column_names(a->table);
    for (i = 0; i < na; ++i) {
      name = cpl_array_get_string(names, i);
      cpl_error_ensure(name != NULL, cpl_error_get_code(), break,
                  "Failed to get the name for column %"CPL_SIZE_FORMAT".", i);
      if (! cpl_table_has_column(b->table, name)
          || ! _irplib_table_column_equal(a->table, b->table, name, CPL_FALSE))
      {
        no_match = CPL_TRUE;
        break;
      }
    }
    cpl_array_delete(names);
    /* Check that no errors occurred and all columns were processed. */
    if (no_match || ! cpl_errorstate_is_equal(prestate)) return CPL_FALSE;
  }

  /* If we got to this point then all checks passed, so return true (a == b). */
  return CPL_TRUE;
}

/*----------------------------------------------------------------------------*/
/**
 * @internal
 * @brief Counts the number of keywords matching a given regular expression.
 */
/*----------------------------------------------------------------------------*/
static cpl_size
_irplib_sdp_spectrum_count_keywords(const irplib_sdp_spectrum *self,
                                    const char *regexp)
{
  cpl_error_code error;
  cpl_size result = 0;
  cpl_propertylist *list = cpl_propertylist_new();

  assert(self != NULL);
  assert(self->proplist != NULL);

  error = cpl_propertylist_copy_property_regexp(list, self->proplist, regexp,
                                                CPL_FALSE);
  if (! error) {
    result = cpl_propertylist_get_size(list);
  }
  cpl_propertylist_delete(list);
  return result;
}


cpl_size irplib_sdp_spectrum_count_obid(const irplib_sdp_spectrum *self)
{
  cpl_ensure(self != NULL, CPL_ERROR_NULL_INPUT, 0);
  return _irplib_sdp_spectrum_count_keywords(self, "^OBID[0-9]+$");
}

cpl_size irplib_sdp_spectrum_count_prov(const irplib_sdp_spectrum *self)
{
  cpl_ensure(self != NULL, CPL_ERROR_NULL_INPUT, 0);
  return _irplib_sdp_spectrum_count_keywords(self, "^PROV[0-9]+$");
}

cpl_size irplib_sdp_spectrum_count_asson(const irplib_sdp_spectrum *self)
{
  cpl_ensure(self != NULL, CPL_ERROR_NULL_INPUT, 0);
  return _irplib_sdp_spectrum_count_keywords(self, "^ASSON[0-9]+$");
}

cpl_size irplib_sdp_spectrum_count_assoc(const irplib_sdp_spectrum *self)
{
  cpl_ensure(self != NULL, CPL_ERROR_NULL_INPUT, 0);
  return _irplib_sdp_spectrum_count_keywords(self, "^ASSOC[0-9]+$");
}

cpl_size irplib_sdp_spectrum_count_assom(const irplib_sdp_spectrum *self)
{
  cpl_ensure(self != NULL, CPL_ERROR_NULL_INPUT, 0);
  return _irplib_sdp_spectrum_count_keywords(self, "^ASSOM[0-9]+$");
}


#ifndef NDEBUG

/**
 * @internal
 * @brief Check if keyword table is sorted and has unique entries.
 */
static cpl_boolean _irplib_keyword_table_is_sorted(
                            const irplib_keyword_record *table, size_t entries)
{
  size_t i;
  if (entries < 2) return CPL_TRUE;
  for (i = 0; i < entries-1; ++i) {
    if (strcmp(table[i].name, table[i+1].name) >= 0) {
      return CPL_FALSE;
    }
  }
  return CPL_TRUE;
}

#endif /* NDEBUG */


static const irplib_keyword_record *
_irplib_sdp_spectrum_get_keyword_record(const char *name)
{
  /* The following table should contain all valid SDP spectrum keywords being
   * handled. NOTE: this table must be kept sorted since we perform a binary
   * search on the first column (i.e. the keyword name). */
  static const irplib_keyword_record keyword_table[] = {
  /*    Name              Comment              Type         Is an array key */
    {KEY_APERTURE,  KEY_APERTURE_COMMENT, CPL_TYPE_DOUBLE,   CPL_FALSE},
    {KEY_ASSOC,     KEY_ASSOC_COMMENT,    CPL_TYPE_STRING,   CPL_TRUE},
    {KEY_ASSOM,     KEY_ASSOM_COMMENT,    CPL_TYPE_STRING,   CPL_TRUE},
    {KEY_ASSON,     KEY_ASSON_COMMENT,    CPL_TYPE_STRING,   CPL_TRUE},
    {KEY_CONTNORM,  KEY_CONTNORM_COMMENT, CPL_TYPE_BOOL,     CPL_FALSE},
    {KEY_DEC,       KEY_DEC_COMMENT,      CPL_TYPE_DOUBLE,   CPL_FALSE},
    {KEY_DETRON,    KEY_DETRON_COMMENT,   CPL_TYPE_DOUBLE,   CPL_FALSE},
    {KEY_DISPELEM,  KEY_DISPELEM_COMMENT, CPL_TYPE_STRING,   CPL_FALSE},
    {KEY_EFFRON,    KEY_EFFRON_COMMENT,   CPL_TYPE_DOUBLE,   CPL_FALSE},
    {KEY_EXPTIME,   KEY_EXPTIME_COMMENT,  CPL_TYPE_DOUBLE,   CPL_FALSE},
    {KEY_EXTNAME,   KEY_EXTNAME_COMMENT,  CPL_TYPE_STRING,   CPL_FALSE},
    {KEY_EXT_OBJ,   KEY_EXT_OBJ_COMMENT,  CPL_TYPE_BOOL,     CPL_FALSE},
    {KEY_FLUXCAL,   KEY_FLUXCAL_COMMENT,  CPL_TYPE_STRING,   CPL_FALSE},
    {KEY_FLUXERR,   KEY_FLUXERR_COMMENT,  CPL_TYPE_DOUBLE,   CPL_FALSE},
    {KEY_GAIN,      KEY_GAIN_COMMENT,     CPL_TYPE_DOUBLE,   CPL_FALSE},
    {KEY_INHERIT,   KEY_INHERIT_COMMENT,  CPL_TYPE_BOOL,     CPL_FALSE},
    {KEY_LAMNLIN,   KEY_LAMNLIN_COMMENT,  CPL_TYPE_INT,      CPL_FALSE},
    {KEY_LAMRMS,    KEY_LAMRMS_COMMENT,   CPL_TYPE_DOUBLE,   CPL_FALSE},
    {KEY_MJDEND,    KEY_MJDEND_COMMENT,   CPL_TYPE_DOUBLE,   CPL_FALSE},
    {KEY_MJDOBS,    KEY_MJDOBS_COMMENT,   CPL_TYPE_DOUBLE,   CPL_FALSE},
    {KEY_M_EPOCH,   KEY_M_EPOCH_COMMENT,  CPL_TYPE_BOOL,     CPL_FALSE},
    {KEY_NCOMBINE,  KEY_NCOMBINE_COMMENT, CPL_TYPE_INT,      CPL_FALSE},
    {KEY_NELEM,     KEY_NELEM_COMMENT,    IRPLIB_TYPE_NELEM, CPL_FALSE},
    {KEY_OBID,      KEY_OBID_COMMENT,     CPL_TYPE_INT,      CPL_TRUE},
    {KEY_OBJECT,    KEY_OBJECT_COMMENT,   CPL_TYPE_STRING,   CPL_FALSE},
    {KEY_OBSTECH,   KEY_OBSTECH_COMMENT,  CPL_TYPE_STRING,   CPL_FALSE},
    {KEY_ORIGIN,    KEY_ORIGIN_COMMENT,   CPL_TYPE_STRING,   CPL_FALSE},
    {KEY_PROCSOFT,  KEY_PROCSOFT_COMMENT, CPL_TYPE_STRING,   CPL_FALSE},
    {KEY_PRODCATG,  KEY_PRODCATG_COMMENT, CPL_TYPE_STRING,   CPL_FALSE},
    {KEY_PRODLVL,   KEY_PRODLVL_COMMENT,  CPL_TYPE_INT,      CPL_FALSE},
    {KEY_PROG_ID,   KEY_PROG_ID_COMMENT,  CPL_TYPE_STRING,   CPL_FALSE},
    {KEY_PROV,      KEY_PROV_COMMENT,     CPL_TYPE_STRING,   CPL_TRUE},
    {KEY_RA,        KEY_RA_COMMENT,       CPL_TYPE_DOUBLE,   CPL_FALSE},
    {KEY_REFERENC,  KEY_REFERENC_COMMENT, CPL_TYPE_STRING,   CPL_FALSE},
    {KEY_SNR,       KEY_SNR_COMMENT,      CPL_TYPE_DOUBLE,   CPL_FALSE},
    {KEY_SPECSYS,   KEY_SPECSYS_COMMENT,  CPL_TYPE_DOUBLE,   CPL_FALSE},
    {KEY_SPEC_BIN,  KEY_SPEC_BIN_COMMENT, CPL_TYPE_DOUBLE,   CPL_FALSE},
    {KEY_SPEC_BW,   KEY_SPEC_BW_COMMENT,  CPL_TYPE_DOUBLE,   CPL_FALSE},
    {KEY_SPEC_ERR,  KEY_SPEC_ERR_COMMENT, CPL_TYPE_DOUBLE,   CPL_FALSE},
    {KEY_SPEC_RES,  KEY_SPEC_RES_COMMENT, CPL_TYPE_DOUBLE,   CPL_FALSE},
    {KEY_SPEC_SYE,  KEY_SPEC_SYE_COMMENT, CPL_TYPE_DOUBLE,   CPL_FALSE},
    {KEY_SPEC_VAL,  KEY_SPEC_VAL_COMMENT, CPL_TYPE_DOUBLE,   CPL_FALSE},
    {KEY_TCOMM,     KEY_TCOMM_COMMENT,    CPL_TYPE_STRING,   CPL_TRUE},
    {KEY_TDMAX(1),  KEY_TDMAX1_COMMENT,   CPL_TYPE_DOUBLE,   CPL_FALSE},
    {KEY_TDMIN(1),  KEY_TDMIN1_COMMENT,   CPL_TYPE_DOUBLE,   CPL_FALSE},
    {KEY_TELAPSE,   KEY_TELAPSE_COMMENT,  CPL_TYPE_DOUBLE,   CPL_FALSE},
    {KEY_TEXPTIME,  KEY_TEXPTIME_COMMENT, CPL_TYPE_DOUBLE,   CPL_FALSE},
    {KEY_TIMESYS,   KEY_TIMESYS_COMMENT,  CPL_TYPE_STRING,   CPL_FALSE},
    {KEY_TITLE,     KEY_TITLE_COMMENT,    CPL_TYPE_STRING,   CPL_FALSE},
    {KEY_TMID,      KEY_TMID_COMMENT,     CPL_TYPE_DOUBLE,   CPL_FALSE},
    {KEY_TOT_FLUX,  KEY_TOT_FLUX_COMMENT, CPL_TYPE_BOOL,     CPL_FALSE},
    {KEY_TUCD,      KEY_TUCD_COMMENT,     CPL_TYPE_STRING,   CPL_TRUE},
    {KEY_TUTYP,     KEY_TUTYP_COMMENT,    CPL_TYPE_STRING,   CPL_TRUE},
    {KEY_VOCLASS,   KEY_VOCLASS_COMMENT,  CPL_TYPE_STRING,   CPL_FALSE},
    {KEY_VOPUB,     KEY_VOPUB_COMMENT,    CPL_TYPE_STRING,   CPL_FALSE},
    {KEY_WAVELMAX,  KEY_WAVELMAX_COMMENT, CPL_TYPE_DOUBLE,   CPL_FALSE},
    {KEY_WAVELMIN,  KEY_WAVELMIN_COMMENT, CPL_TYPE_DOUBLE,   CPL_FALSE}
  };

  static const size_t tablesize =
                        sizeof(keyword_table) / sizeof(irplib_keyword_record);
  size_t low = 0;             /* Low end of search region. */
  size_t high = tablesize-1;  /* High end of search region. */
  const irplib_keyword_record *record = NULL;

  assert(_irplib_keyword_table_is_sorted(keyword_table, tablesize));
  assert(name != NULL);

  /* Binary search for the keyword record who's name forms the prefix of the
   * 'name' string passed to this function or is equal to that string. We cannot
   * just check if they equal since the OBIDi, PROVi, ASSONi, ASSOCi, ASSOMi,
   * TUTYPi and TUCDi keywords all have a number suffix that needs to be dealt
   * with. */
  do {
    size_t mid = (low + high) >> 1;   /* Find mid point between low and high. */
    size_t keylen = strlen(keyword_table[mid].name);
    int result = strncmp(name, keyword_table[mid].name, keylen);
    if (result == 0) {
      record = &keyword_table[mid];
      break;
    } else if (result < 0) {
      if (mid >= 1) {
        high = mid - 1;
      } else {
        return NULL;
      }
    } else {
      low = mid + 1;
      if (low > high) return NULL;
    }
  } while (1);

  assert(record != NULL);

  if (strlen(record->name) != strlen(name)) {
    if (! record->is_array_key) return NULL;
    /* Have to check if the keyword format is correct. Should only have digits
     * following the name prefix. */
    const char *c = name + strlen(record->name);
    while (*c != '\0') {
      if (! isdigit(*c)) return NULL;
      ++c;
    }
  }

  return record;
}


cpl_error_code irplib_sdp_spectrum_copy_keyword(irplib_sdp_spectrum *self,
                                                const cpl_propertylist *plist,
                                                const char *name)
{
  const irplib_keyword_record *key;
  cpl_errorstate prestate = cpl_errorstate_get();
  cpl_boolean spectrum_has_keyword;

  cpl_ensure_code(self != NULL && plist != NULL && name != NULL,
                  CPL_ERROR_NULL_INPUT);

  assert(self->proplist != NULL);

  if (! cpl_propertylist_has(plist, name)) {
    return cpl_error_set_message(cpl_func, cpl_error_get_code(),
                    "Could not set '%s' since the keyword was not found in the"
                    " source list.", name);
  }

  key = _irplib_sdp_spectrum_get_keyword_record(name);
  if (key == NULL) {
    return cpl_error_set_message(cpl_func, CPL_ERROR_ILLEGAL_INPUT,
                    "The keyword name '%s' is not valid for an SPD spectrum.",
                    name);
  }

  spectrum_has_keyword = cpl_propertylist_has(self->proplist, name);

  switch ((int) key->type) {
  case CPL_TYPE_BOOL:
    {
      /* Note: we update with the following functions rather than using the
       * cpl_propertylist_copy_property function since this way we get basic
       * typecasting functionality. e.g. floats get converted to doubles. */
      cpl_boolean value = cpl_propertylist_get_bool(plist, name);
      cpl_propertylist_update_bool(self->proplist, name, value);
    }
    break;
  case CPL_TYPE_INT:
    {
      int value = cpl_propertylist_get_int(plist, name);
      cpl_propertylist_update_int(self->proplist, name, value);
    }
    break;
  case CPL_TYPE_DOUBLE:
    {
      double value = cpl_propertylist_get_double(plist, name);
      cpl_propertylist_update_double(self->proplist, name, value);
    }
    break;
  case CPL_TYPE_STRING:
    {
      const char *value = cpl_propertylist_get_string(plist, name);
      cpl_propertylist_update_string(self->proplist, name, value);
    }
    break;
  case IRPLIB_TYPE_NELEM:
    {
      /* Special case where we update the nelem field. */
      spectrum_has_keyword = CPL_TRUE;  /* Skip trying to set comment. */
      cpl_size value = (cpl_size) cpl_propertylist_get_long_long(plist, name);
      if (cpl_errorstate_is_equal(prestate)) {
        irplib_sdp_spectrum_set_nelem(self, value);
      }
    }
    break;
  default:
    return cpl_error_set_message(cpl_func, CPL_ERROR_INVALID_TYPE,
                      "Cannot handle type '%s'.", cpl_type_get_name(key->type));
  }

  if (! spectrum_has_keyword) {
    cpl_propertylist_set_comment(self->proplist, name, key->comment);
  }

  if (! cpl_errorstate_is_equal(prestate)) {
    if (! spectrum_has_keyword) {
      /* Make sure the keyword is removed if we have an error and it was not
       * there to begin with. */
      prestate = cpl_errorstate_get();
      (void) cpl_propertylist_erase(self->proplist, name);
      cpl_errorstate_set(prestate);
    }
    return cpl_error_set_message(cpl_func, cpl_error_get_code(),
                  "Could not set '%s'. Likely the keyword from the source list"
                  " has a different format or type.", name);
  }

  return CPL_ERROR_NONE;
}


cpl_error_code irplib_sdp_spectrum_copy_property(irplib_sdp_spectrum *self,
                                                 const cpl_property *prop)
{
  const char *name;
  const irplib_keyword_record *key;
  cpl_errorstate prestate = cpl_errorstate_get();
  cpl_boolean spectrum_has_keyword;

  cpl_ensure_code(self != NULL, CPL_ERROR_NULL_INPUT);

  assert(self->proplist != NULL);

  name = cpl_property_get_name(prop);
  if (name == NULL) return cpl_error_get_code();

  key = _irplib_sdp_spectrum_get_keyword_record(name);
  if (key == NULL) {
    return cpl_error_set_message(cpl_func, CPL_ERROR_ILLEGAL_INPUT,
                    "The keyword name '%s' is not valid for an SPD spectrum.",
                    name);
  }

  spectrum_has_keyword = cpl_propertylist_has(self->proplist, name);

  switch ((int) key->type) {
  case CPL_TYPE_BOOL:
    {
      cpl_boolean value = cpl_property_get_bool(prop);
      cpl_propertylist_update_bool(self->proplist, name, value);
    }
    break;
  case CPL_TYPE_INT:
    {
      int value = cpl_property_get_int(prop);
      cpl_propertylist_update_int(self->proplist, name, value);
    }
    break;
  case CPL_TYPE_DOUBLE:
    {
      double value = cpl_property_get_double(prop);
      cpl_propertylist_update_double(self->proplist, name, value);
    }
    break;
  case CPL_TYPE_STRING:
    {
      const char *value = cpl_property_get_string(prop);
      cpl_propertylist_update_string(self->proplist, name, value);
    }
    break;
  case IRPLIB_TYPE_NELEM:
    {
      /* Special case where we update the nelem field. */
      spectrum_has_keyword = CPL_TRUE;  /* Skip trying to set comment. */
      cpl_size value = (cpl_size) cpl_property_get_long_long(prop);
      if (cpl_errorstate_is_equal(prestate)) {
        irplib_sdp_spectrum_set_nelem(self, value);
      }
    }
    break;
  default:
    return cpl_error_set_message(cpl_func, CPL_ERROR_INVALID_TYPE,
                      "Cannot handle type '%s'.", cpl_type_get_name(key->type));
  }

  if (! spectrum_has_keyword) {
    cpl_propertylist_set_comment(self->proplist, name, key->comment);
  }

  if (! cpl_errorstate_is_equal(prestate)) {
    if (! spectrum_has_keyword) {
      /* Make sure the keyword is removed if we have an error and it was not
       * there to begin with. */
      prestate = cpl_errorstate_get();
      (void) cpl_propertylist_erase(self->proplist, name);
      cpl_errorstate_set(prestate);
    }
    return cpl_error_set_message(cpl_func, cpl_error_get_code(),
                      "Could not set '%s'. Likely the source property has a"
                      " different format or type.", name);
  }

  return CPL_ERROR_NONE;
}


cpl_error_code irplib_sdp_spectrum_copy_property_regexp(
                                                irplib_sdp_spectrum *self,
                                                const cpl_propertylist *plist,
                                                const char *regexp,
                                                int invert)
{
  cpl_propertylist *sublist = NULL;
  cpl_propertylist *origlist = NULL;
  cpl_errorstate prestate = cpl_errorstate_get();
  cpl_size i;

  cpl_ensure_code(self != NULL, CPL_ERROR_NULL_INPUT);

  assert(self->proplist != NULL);

  sublist = cpl_propertylist_new();
  origlist = cpl_propertylist_new();
  cpl_propertylist_copy_property_regexp(origlist, self->proplist, regexp,
                                        invert);
  cpl_propertylist_copy_property_regexp(sublist, plist, regexp, invert);
  if (cpl_propertylist_has(sublist, KEY_NELEM)) {
    /* Move the NELEM key to the end of the list so that rollback on error is
     * easier. */
    cpl_propertylist_erase(sublist, KEY_NELEM);
    cpl_propertylist_copy_property(sublist, plist, KEY_NELEM);
  }
  if (! cpl_errorstate_is_equal(prestate)) goto cleanup;

  for (i = 0; i < cpl_propertylist_get_size(sublist); ++i) {
    const cpl_property *p = cpl_propertylist_get_const(sublist, i);
    const char *name = cpl_property_get_name(p);
    irplib_sdp_spectrum_copy_keyword(self, sublist, name);
    if (! cpl_errorstate_is_equal(prestate)) goto cleanup;
  }

  cpl_propertylist_delete(sublist);
  cpl_propertylist_delete(origlist);
  return CPL_ERROR_NONE;

cleanup:
  /* Cleanup here if an error occurred, by restoring the keywords to the
   * original values. */
  prestate = cpl_errorstate_get();
  (void) cpl_propertylist_copy_property_regexp(self->proplist, origlist,
                                               ".*", 0);
  cpl_errorstate_set(prestate);
  cpl_propertylist_delete(sublist);
  cpl_propertylist_delete(origlist);
  return cpl_error_get_code();
}


GET_SET_METHODS_TYPE_DOUBLE(ra, KEY_RA, KEY_RA_COMMENT)
GET_SET_METHODS_TYPE_DOUBLE(dec, KEY_DEC, KEY_DEC_COMMENT)
GET_SET_METHODS_TYPE_DOUBLE(exptime, KEY_EXPTIME, KEY_EXPTIME_COMMENT)
GET_SET_METHODS_TYPE_DOUBLE(texptime, KEY_TEXPTIME, KEY_TEXPTIME_COMMENT)
GET_SET_METHODS_TYPE_STRING(timesys, KEY_TIMESYS, KEY_TIMESYS_COMMENT)
GET_SET_METHODS_TYPE_DOUBLE(mjdobs, KEY_MJDOBS, KEY_MJDOBS_COMMENT)
GET_SET_METHODS_TYPE_DOUBLE(mjdend, KEY_MJDEND, KEY_MJDEND_COMMENT)
GET_SET_METHODS_TYPE_INT(prodlvl, KEY_PRODLVL, KEY_PRODLVL_COMMENT)
GET_SET_METHODS_TYPE_STRING(procsoft, KEY_PROCSOFT, KEY_PROCSOFT_COMMENT)
GET_SET_METHODS_TYPE_STRING(prodcatg, KEY_PRODCATG, KEY_PRODCATG_COMMENT)
GET_SET_METHODS_TYPE_STRING(origin, KEY_ORIGIN, KEY_ORIGIN_COMMENT)
GET_SET_METHODS_TYPE_BOOL(extobj, KEY_EXT_OBJ, KEY_EXT_OBJ_COMMENT)
GET_SET_METHODS_TYPE_STRING(dispelem, KEY_DISPELEM, KEY_DISPELEM_COMMENT)
GET_SET_METHODS_TYPE_STRING(specsys, KEY_SPECSYS, KEY_SPECSYS_COMMENT)
GET_SET_METHODS_TYPE_STRING(progid, KEY_PROG_ID, KEY_PROG_ID_COMMENT)
GET_SET_ARRAY_METHODS_TYPE_INT(obid, KEY_OBID, KEY_OBID_COMMENT)
GET_SET_METHODS_TYPE_BOOL(mepoch, KEY_M_EPOCH, KEY_M_EPOCH_COMMENT)
GET_SET_METHODS_TYPE_STRING(obstech, KEY_OBSTECH, KEY_OBSTECH_COMMENT)
GET_SET_METHODS_TYPE_STRING(fluxcal, KEY_FLUXCAL, KEY_FLUXCAL_COMMENT)
GET_SET_METHODS_TYPE_BOOL(contnorm, KEY_CONTNORM, KEY_CONTNORM_COMMENT)
GET_SET_METHODS_TYPE_DOUBLE(wavelmin, KEY_WAVELMIN, KEY_WAVELMIN_COMMENT)
GET_SET_METHODS_TYPE_DOUBLE(wavelmax, KEY_WAVELMAX, KEY_WAVELMAX_COMMENT)
GET_SET_METHODS_TYPE_DOUBLE(specbin, KEY_SPEC_BIN, KEY_SPEC_BIN_COMMENT)
GET_SET_METHODS_TYPE_BOOL(totflux, KEY_TOT_FLUX, KEY_TOT_FLUX_COMMENT)
GET_SET_METHODS_TYPE_DOUBLE(fluxerr, KEY_FLUXERR, KEY_FLUXERR_COMMENT)
GET_SET_METHODS_TYPE_STRING(referenc, KEY_REFERENC, KEY_REFERENC_COMMENT)
GET_SET_METHODS_TYPE_DOUBLE(specres, KEY_SPEC_RES, KEY_SPEC_RES_COMMENT)
GET_SET_METHODS_TYPE_DOUBLE(specerr, KEY_SPEC_ERR, KEY_SPEC_ERR_COMMENT)
GET_SET_METHODS_TYPE_DOUBLE(specsye, KEY_SPEC_SYE, KEY_SPEC_SYE_COMMENT)
GET_SET_METHODS_TYPE_INT(lamnlin, KEY_LAMNLIN, KEY_LAMNLIN_COMMENT)
GET_SET_METHODS_TYPE_DOUBLE(lamrms, KEY_LAMRMS, KEY_LAMRMS_COMMENT)
GET_SET_METHODS_TYPE_DOUBLE(gain, KEY_GAIN, KEY_GAIN_COMMENT)
GET_SET_METHODS_TYPE_DOUBLE(detron, KEY_DETRON, KEY_DETRON_COMMENT)
GET_SET_METHODS_TYPE_DOUBLE(effron, KEY_EFFRON, KEY_EFFRON_COMMENT)
GET_SET_METHODS_TYPE_DOUBLE(snr, KEY_SNR, KEY_SNR_COMMENT)
GET_SET_METHODS_TYPE_INT(ncombine, KEY_NCOMBINE, KEY_NCOMBINE_COMMENT)
GET_SET_ARRAY_METHODS_TYPE_STRING(prov, KEY_PROV, KEY_PROV_COMMENT)
GET_SET_ARRAY_METHODS_TYPE_STRING(asson, KEY_ASSON, KEY_ASSON_COMMENT)
GET_SET_ARRAY_METHODS_TYPE_STRING(assoc, KEY_ASSOC, KEY_ASSOC_COMMENT)
GET_SET_ARRAY_METHODS_TYPE_STRING(assom, KEY_ASSOM, KEY_ASSOM_COMMENT)
GET_SET_METHODS_TYPE_STRING(voclass, KEY_VOCLASS, KEY_VOCLASS_COMMENT)
GET_SET_METHODS_TYPE_STRING(vopub, KEY_VOPUB, KEY_VOPUB_COMMENT)
GET_SET_METHODS_TYPE_STRING(title, KEY_TITLE, KEY_TITLE_COMMENT)
GET_SET_METHODS_TYPE_STRING(object, KEY_OBJECT, KEY_OBJECT_COMMENT)
GET_SET_METHODS_TYPE_DOUBLE(aperture, KEY_APERTURE, KEY_APERTURE_COMMENT)
GET_SET_METHODS_TYPE_DOUBLE(telapse, KEY_TELAPSE, KEY_TELAPSE_COMMENT)
GET_SET_METHODS_TYPE_DOUBLE(tmid, KEY_TMID, KEY_TMID_COMMENT)
GET_SET_METHODS_TYPE_DOUBLE(specval, KEY_SPEC_VAL, KEY_SPEC_VAL_COMMENT)
GET_SET_METHODS_TYPE_DOUBLE(specbw, KEY_SPEC_BW, KEY_SPEC_BW_COMMENT)
GET_SET_METHODS_TYPE_STRING(extname, KEY_EXTNAME, KEY_EXTNAME_COMMENT)
GET_SET_METHODS_TYPE_BOOL(inherit, KEY_INHERIT, KEY_INHERIT_COMMENT)
GET_SET_METHODS_TYPE_DOUBLE(tdmin, KEY_TDMIN(1), KEY_TDMIN1_COMMENT)
GET_SET_METHODS_TYPE_DOUBLE(tdmax, KEY_TDMAX(1), KEY_TDMAX1_COMMENT)


#if 0
/*----------------------------------------------------------------------------*/
/**
 * @brief Replace a keyword comment by a custom string.
 * @param self    The spectrum object to update.
 * @param keyword The name of the keyword to update.
 * @param comment The string to use as keyword comment.
 * @return @c CPL_ERROR_NONE on success or an appropriate error code otherwise.
 *
 * This function will replace the comment of the keyword @c keyword with
 * the string @c comment. The string @c keyword is the full keyword name.
 * Both, the keyword @c keyword and the new comment @c comment must not
 * be @a NULL.
 * If any error occurs then a error code is set and returned, otherwise
 * @c CPL_ERROR_NONE is returned on success.
 */
/*----------------------------------------------------------------------------*/

cpl_error_code
irplib_sdp_spectrum_replace_comment(irplib_sdp_spectrum *self,
                                    const char *keyword,
                                    const char *comment)
{
    cpl_ensure_code((self != NULL) && (keyword != NULL) && (comment != NULL),
                    CPL_ERROR_NULL_INPUT);
    cpl_ensure_code((self->proplist != NULL), CPL_ERROR_ILLEGAL_INPUT);

    if (!cpl_propertylist_has(self->proplist, keyword)) {
        return cpl_error_set_message(cpl_func, CPL_ERROR_DATA_NOT_FOUND,
                    "Could not find '%s' keyword.", keyword);
    }

    cpl_propertylist_set_comment(self->proplist, keyword, comment);
    return CPL_ERROR_NONE;
}
#endif


cpl_error_code irplib_sdp_spectrum_append_prov(irplib_sdp_spectrum *self,
                                               cpl_size firstindex,
                                               const cpl_frameset *frames)
{
  cpl_frameset_iterator* iter = NULL;
  cpl_propertylist* keywords = NULL;
  const cpl_frame* frame;
  cpl_size index = firstindex;

  /* Note: check for NULL already done in irplib_sdp_spectrum_set_prov below. */
  assert(self != NULL);
  assert(self->proplist != NULL);

  iter = cpl_frameset_iterator_new(frames);
  frame = cpl_frameset_iterator_get_const(iter);
  while (frame != NULL) {
    cpl_error_code error;
    const char* value = NULL;

    /* Load the keywords from the raw frame. */
    const char* filename = cpl_frame_get_filename(frame);
    cpl_error_ensure(filename != NULL, cpl_error_get_code(), goto cleanup,
                     "%s", cpl_error_get_message());
    keywords = cpl_propertylist_load(filename, 0);
    cpl_error_ensure(filename != NULL, cpl_error_get_code(), goto cleanup,
                     "Could not load keywords from primary HDU in '%s'.",
                     filename);

    /* Try set the value to ARCFILE or ORIGFILE or just the filename, whichever
     * is found first in that order. */
    if (cpl_propertylist_has(keywords, KEY_ARCFILE)) {
      value = cpl_propertylist_get_string(keywords, KEY_ARCFILE);
      cpl_error_ensure(value != NULL, cpl_error_get_code(), goto cleanup,
                       "Could not extract the '%s' keyword value from '%s'.",
                       KEY_ARCFILE, filename);
    } else if (cpl_propertylist_has(keywords, KEY_ORIGFILE)) {
      value = cpl_propertylist_get_string(keywords, KEY_ORIGFILE);
      cpl_error_ensure(value != NULL, cpl_error_get_code(), goto cleanup,
                       "Could not extract the '%s' keyword value from '%s'.",
                       KEY_ORIGFILE, filename);
    } else {
      value = filename;
    }

    /* Add the next PROVi keyword. */
    error = irplib_sdp_spectrum_set_prov(self, index, value);
    cpl_error_ensure(! error, error, goto cleanup,
                     "%s", cpl_error_get_message());
    cpl_propertylist_delete(keywords);
    keywords = NULL;

    /* Increment the iterator to the next frame. */
    cpl_errorstate status = cpl_errorstate_get();
    cpl_frameset_iterator_advance(iter, 1);
    if (cpl_error_get_code() == CPL_ERROR_ACCESS_OUT_OF_RANGE) {
        cpl_errorstate_set(status);
    }
    frame = cpl_frameset_iterator_get_const(iter);
    ++index;
  }

  cpl_frameset_iterator_delete(iter);
  return CPL_ERROR_NONE;

cleanup:
  /* Cleanup if an error occurs. Note: delete methods already check for NULL. */
  cpl_frameset_iterator_delete(iter);
  cpl_propertylist_delete(keywords);
  return cpl_error_get_code();
}


cpl_size irplib_sdp_spectrum_get_nelem(const irplib_sdp_spectrum *self)
{
  cpl_ensure(self != NULL, CPL_ERROR_NULL_INPUT, 0);
  return self->nelem;
}


cpl_error_code irplib_sdp_spectrum_reset_nelem(irplib_sdp_spectrum *self)
{
  return irplib_sdp_spectrum_set_nelem(self, 0);
}


cpl_error_code irplib_sdp_spectrum_set_nelem(irplib_sdp_spectrum *self,
                                             cpl_size value)
{
  cpl_size ncol;
  cpl_error_code error = CPL_ERROR_NONE;

  cpl_ensure(self != NULL, CPL_ERROR_NULL_INPUT, 0);

  assert(self->table != NULL);

  ncol = cpl_table_get_ncol(self->table);
  if (ncol > 0) {
    /* Update all column depths. */
    cpl_size i;
    cpl_array *names = cpl_table_get_column_names(self->table);
    for (i = 0; i < ncol; ++i) {
      const char *name = cpl_array_get_string(names, i);
      error = cpl_table_set_column_depth(self->table, name, value);
      if (error) {
        /* If an error occurs then set the columns that were changed back to
         * the previous value. */
        cpl_size j;
        cpl_errorstate prestate = cpl_errorstate_get();
        for (j = 0; j < i; ++j) {
          (void) cpl_table_set_column_depth(self->table, name, self->nelem);
        }
        cpl_errorstate_set(prestate);
        break;
      }
    }
    cpl_array_delete(names);
  }
  if (! error) {
    self->nelem = value;
  }
  return error;
}


cpl_error_code irplib_sdp_spectrum_copy_nelem(irplib_sdp_spectrum *self,
                                              const cpl_propertylist *plist,
                                              const char *name)
{
  /* Note: check for plist or name == NULL is done in cpl_propertylist calls. */
  cpl_ensure_code(self != NULL, CPL_ERROR_NULL_INPUT);

  assert(self->proplist != NULL);

  if (cpl_propertylist_has(plist, name)) {
    cpl_errorstate prestate = cpl_errorstate_get();
    cpl_size value = (cpl_size) cpl_propertylist_get_long_long(plist, name);
    if (cpl_errorstate_is_equal(prestate)) {
      return irplib_sdp_spectrum_set_nelem(self, value);
    } else {
      return cpl_error_set_message(cpl_func, cpl_error_get_code(),
                "Could not set '%s'. Likely the source '%s' keyword has a"
                " different format or type.", KEY_NELEM, name);
    }
  } else {
    return cpl_error_set_message(cpl_func, CPL_ERROR_DATA_NOT_FOUND,
                "Could not set '%s' since the '%s' keyword was not found.",
                KEY_NELEM, name);
  }
}


cpl_size irplib_sdp_spectrum_get_ncol(const irplib_sdp_spectrum *self)
{
  cpl_ensure(self != NULL, CPL_ERROR_NULL_INPUT, 0);
  assert(self->table != NULL);
  return cpl_table_get_ncol(self->table);
}


cpl_boolean irplib_sdp_spectrum_has_column(const irplib_sdp_spectrum *self,
                                           const char* name)
{
  cpl_ensure(self != NULL, CPL_ERROR_NULL_INPUT, 0);
  assert(self->table != NULL);
  return cpl_table_has_column(self->table, name);
}


cpl_array *
irplib_sdp_spectrum_get_column_names(const irplib_sdp_spectrum *self)
{
  cpl_ensure(self != NULL, CPL_ERROR_NULL_INPUT, NULL);
  assert(self->table != NULL);
  return cpl_table_get_column_names(self->table);
}


cpl_error_code
irplib_sdp_spectrum_new_column(irplib_sdp_spectrum *self, const char *name,
                               cpl_type type)
{
  cpl_error_code error;
  cpl_ensure_code(self != NULL, CPL_ERROR_NULL_INPUT);
  assert(self->table != NULL);
  error = cpl_table_new_column_array(self->table, name, type, self->nelem);
  if (error) {
    cpl_error_set_message(cpl_func, cpl_error_get_code(),
                          "Failed to create a new column called '%s'.", name);
  }
  return error;
}


cpl_error_code
irplib_sdp_spectrum_add_column(irplib_sdp_spectrum *self, const char *name,
                               cpl_type type, const char *unit,
                               const char *format, const char *tutyp,
                               const char *tucd, const cpl_array *data)
{
  cpl_error_code error;

  /* Note: check for name equals NULL should already be done in table calls. */
  cpl_ensure_code(self != NULL, CPL_ERROR_NULL_INPUT);

  assert(self->table != NULL);

  /* Setup a new array cell column and fill its properties (possibly with
   * defaults). */
  error = cpl_table_new_column_array(self->table, name, type, self->nelem);
  if (unit != NULL && *unit != '\0') {
    error |= cpl_table_set_column_unit(self->table, name, unit);
  } else {
    error |= cpl_table_set_column_unit(self->table, name, " ");
  }
  if (format != NULL) {
    error |= cpl_table_set_column_format(self->table, name, format);
  }
  if (tutyp != NULL) {
    error |= irplib_sdp_spectrum_set_column_tutyp(self, name, tutyp);
  } else {
    error |= irplib_sdp_spectrum_set_column_tutyp(self, name, "");
  }
  if (tucd != NULL) {
    error |= irplib_sdp_spectrum_set_column_tucd(self, name, tucd);
  } else {
    error |= irplib_sdp_spectrum_set_column_tucd(self, name, "");
  }

  /* Fill the table cell with the data array if available, else add an empty
   * array. */
  if (! error) {
    if (data != NULL) {
      error = cpl_table_set_array(self->table, name, 0, data);
    } else {
      cpl_array *array = cpl_array_new(self->nelem, type);
      if (array != NULL) {
        error = cpl_table_set_array(self->table, name, 0, array);
        cpl_array_delete(array);
      } else {
        error = cpl_error_get_code();
      }
    }
  }

  if (error) {
    /* Remove the column just added if there was an error. We initially save and
     * finally restore the error state since we might generate secondary errors
     * when trying to remove the partially created column. But these secondary
     * errors are expected and irrelevant. */
    cpl_errorstate prestate = cpl_errorstate_get();
    _irplib_sdp_spectrum_erase_column_keywords(self, name);
    (void) cpl_table_erase_column(self->table, name);
    cpl_errorstate_set(prestate);
    error = cpl_error_set_message(cpl_func, cpl_error_get_code(),
                            "Failed to create a new column called '%s'.", name);
  }

  return error;
}


cpl_error_code
irplib_sdp_spectrum_delete_column(irplib_sdp_spectrum *self, const char *name)
{
  cpl_errorstate prestate = cpl_errorstate_get();
  cpl_error_code error = CPL_ERROR_NONE;

  cpl_ensure_code(self != NULL && name != NULL, CPL_ERROR_NULL_INPUT);

  assert(self->table != NULL);

  _irplib_sdp_spectrum_erase_column_keywords(self, name);
  if (! cpl_errorstate_is_equal(prestate)) {
    error |= cpl_error_get_code();
  }
  error |= cpl_table_erase_column(self->table, name);
  if (error) {
    return cpl_error_get_code();
  } else {
    return CPL_ERROR_NONE;
  }
}


static cpl_error_code
_irplib_sdp_spectrum_copy_column(irplib_sdp_spectrum *self, const char *to_name,
                                 const cpl_table* table, const char *from_name)
{
  cpl_error_code error;

  assert(self != NULL);
  assert(self->table != NULL);

  error = cpl_table_duplicate_column(self->table, to_name, table, from_name);
  if (error) return error;
  error |= irplib_sdp_spectrum_set_column_tutyp(self, to_name, "");
  error |= irplib_sdp_spectrum_set_column_tucd(self, to_name, "");
  if (error) {
    /* Rollback changes if an error occurred. */
    cpl_errorstate prestate = cpl_errorstate_get();
    _irplib_sdp_spectrum_erase_column_keywords(self, to_name);
    (void) cpl_table_erase_column(self->table, to_name);
    cpl_errorstate_set(prestate);
    return cpl_error_get_code();
  }
  return CPL_ERROR_NONE;
}


cpl_error_code
irplib_sdp_spectrum_copy_column(irplib_sdp_spectrum *self,
                                const cpl_table* table, const char *name)
{
  /* Note: check for table == NULL || name == NULL should already be done in the
   * cpl_table calls. */
  cpl_ensure_code(self != NULL, CPL_ERROR_NULL_INPUT);
  return _irplib_sdp_spectrum_copy_column(self, name, table, name);
}


cpl_error_code
irplib_sdp_spectrum_copy_column_regexp(irplib_sdp_spectrum *self,
                                       const cpl_table* table,
                                       const char *regexp, int invert)
{
  regex_t re;
  cpl_array *names = NULL;
  cpl_size n, i;
  int reg_error_code;

  /* Note: table == NULL is checked in the cpl_table calls. */
  cpl_ensure_code(self != NULL && regexp != NULL, CPL_ERROR_NULL_INPUT);

  assert(self->table != NULL);

  reg_error_code = regcomp(&re, regexp, REG_EXTENDED | REG_NOSUB);
  if (reg_error_code != 0) {
    return cpl_error_set_regex(CPL_ERROR_ILLEGAL_INPUT, reg_error_code, &re,
                               "regexp='%s', invert=%d", regexp, invert);
  }

  /* Go through all column names in the table we are copying from and mark the
   * names the regular expression filters out as invalid. */
  names = cpl_table_get_column_names(table);
  n = cpl_array_get_size(names);
  for (i = 0; i < n; ++i) {
    int match;
    const char *namei = cpl_array_get_string(names, i);
    cpl_error_ensure(! cpl_table_has_column(self->table, namei),
                     CPL_ERROR_ILLEGAL_OUTPUT, goto cleanup,
                     "The column '%s' already exists in the spectrum.", namei);
    match = (regexec(&re, namei, 0, NULL, 0) == 0);
    if ((! match && ! invert) || (match && invert)) {
      cpl_array_set_invalid(names, i);
    }
  }
  /* Now copy only the valid columns. */
  for (i = 0; i < n; ++i) {
    if (cpl_array_is_valid(names, i)) {
      const char *namei = cpl_array_get_string(names, i);
      cpl_error_code error = _irplib_sdp_spectrum_copy_column(self, namei,
                                                              table, namei);
      if (error) {
        cpl_errorstate prestate;
        cpl_size j;
        cpl_error_set_message(cpl_func, error, "Could not copy column '%s'.",
                              namei);
        /* Remove any columns already added if we got an error copying any
         * column. */
        prestate = cpl_errorstate_get();
        for (j = 0; j < i; ++j) {
          namei = cpl_array_get_string(names, i);
          _irplib_sdp_spectrum_erase_column_keywords(self, namei);
          (void) cpl_table_erase_column(self->table, namei);
        }
        cpl_errorstate_set(prestate);
        goto cleanup;
      }
    }
  }
  cpl_array_delete(names);
  regfree(&re);
  return CPL_ERROR_NONE;

cleanup:
  /* This is a cleanup section to delete objects when an error occurs. */
  cpl_array_delete(names);
  regfree(&re);
  return cpl_error_get_code();
}


cpl_error_code
irplib_sdp_spectrum_update_column(irplib_sdp_spectrum *self, const char *name,
                                  const cpl_table* table, const char *colname,
                                  int flags)
{
  char *orig_unit = NULL;
  char *orig_format = NULL;
  cpl_errorstate prestate = cpl_errorstate_get();

  /* Note: check for name, colname equals NULL should already be done in the
   * cpl_table calls. */
  cpl_ensure_code(self != NULL && table != NULL, CPL_ERROR_NULL_INPUT);

  assert(self->table != NULL);

  if (! cpl_table_has_column(self->table, name)) {
    /* The column does not exist in the spectrum so just copy it. */
    return _irplib_sdp_spectrum_copy_column(self, name, table, colname);
  }

  /* Make sure the source column exists. */
  if (! cpl_table_has_column(table, colname)) {
    return cpl_error_set_message(cpl_func, CPL_ERROR_DATA_NOT_FOUND,
                                 "Column '%s' not found in table.", colname);
  }

  /* Update the unit and format values if requested. Note, we copy the original
   * value to be able to restore it if an error occurs. */
  if (flags & IRPLIB_COLUMN_UNIT) {
    const char* unit = cpl_table_get_column_unit(table, colname);
    /* Prevent completely empty strings else cfitsio silently deletes the
     * keyword. */
    if (unit != NULL && *unit == '\0') {
      unit = " ";
    }
    orig_unit = cpl_strdup(cpl_table_get_column_unit(self->table, name));
    cpl_table_set_column_unit(self->table, name, unit);
    if (! cpl_errorstate_is_equal(prestate)) goto cleanup;
  }
  if (flags & IRPLIB_COLUMN_FORMAT) {
    orig_format = cpl_strdup(cpl_table_get_column_format(self->table, name));
    cpl_table_set_column_format(self->table, name,
                                cpl_table_get_column_format(table, colname));
    if (! cpl_errorstate_is_equal(prestate)) goto cleanup;
  }

  /* Update the data array. Leave this to the last task since it is normally
   * cheaper to rollback changes to the unit and format strings if an error
   * occurs. */
  if (flags & IRPLIB_COLUMN_DATA) {
    if (cpl_table_get_column_type(self->table, name) !=
        cpl_table_get_column_type(table, colname)) {
      cpl_error_set_message(cpl_func, CPL_ERROR_INCOMPATIBLE_INPUT,
                    "The table column '%s' and spectrum column '%s' do not"
                    " have the same types.", colname, name);
      goto cleanup;
    }
    if (cpl_table_get_column_depth(self->table, name) !=
        cpl_table_get_column_depth(table, colname)) {
      cpl_error_set_message(cpl_func, CPL_ERROR_INCOMPATIBLE_INPUT,
                    "The table column '%s' and spectrum column '%s' do not"
                    " have the same dimensions.", colname, name);
      goto cleanup;
    }
    const cpl_array* data = cpl_table_get_array(table, colname, 0);
    if (data == NULL) goto cleanup;
    cpl_table_set_array(self->table, name, 0, data);
    if (! cpl_errorstate_is_equal(prestate)) goto cleanup;
  }

  cpl_free(orig_unit);
  cpl_free(orig_format);
  return CPL_ERROR_NONE;

cleanup:
  /* Cleanup if error occurred by rolling back modifications. */
  prestate = cpl_errorstate_get();
  if (orig_unit != NULL) {
    (void) cpl_table_set_column_unit(self->table, name, orig_unit);
    cpl_free(orig_unit);
  }
  if (orig_format != NULL) {
    (void) cpl_table_set_column_format(self->table, name, orig_format);
    cpl_free(orig_format);
  }
  cpl_errorstate_set(prestate);
  return cpl_error_get_code();
}


cpl_type irplib_sdp_spectrum_get_column_type(const irplib_sdp_spectrum *self,
                                             const char *name)
{
  cpl_ensure(self != NULL, CPL_ERROR_NULL_INPUT, CPL_TYPE_INVALID);
  assert(self->table != NULL);
  return cpl_table_get_column_type(self->table, name);
}


const char *
irplib_sdp_spectrum_get_column_unit(const irplib_sdp_spectrum *self,
                                    const char *name)
{
  cpl_ensure(self != NULL, CPL_ERROR_NULL_INPUT, NULL);
  assert(self->table != NULL);
  return cpl_table_get_column_unit(self->table, name);
}

/*----------------------------------------------------------------------------*/
/**
 * @brief Set the physical units for a column.
 * @param self The spectrum object to update.
 * @param name The name of the column to update.
 * @param unit The string indicating the physical units for the column.
 * @return @c CPL_ERROR_NONE on success or an appropriate error code otherwise.
 *
 * This function will set the string indicating the physical units for the
 * column named  by @c name. Valid values for @c unit is @a NULL or a string
 * with at least one character. Empty strings will be implicitly converted to a
 * string with a single space character since CFITSIO does not allow empty
 * strings for the @a TUNIT keywords. If any error occurs then a error code is
 * set and returned, otherwise @c CPL_ERROR_NONE is returned on success.
 */
/*----------------------------------------------------------------------------*/
cpl_error_code
irplib_sdp_spectrum_set_column_unit(irplib_sdp_spectrum *self,
                                    const char *name, const char *unit)
{
  cpl_ensure_code(self != NULL, CPL_ERROR_NULL_INPUT);
  assert(self->table != NULL);
  /* Prevent completely empty strings else cfitsio silently deletes the
   * keyword. */
  if (unit != NULL && *unit == '\0') {
    unit = " ";
  }
  return cpl_table_set_column_unit(self->table, name, unit);
}


cpl_error_code
irplib_sdp_spectrum_copy_column_unit(irplib_sdp_spectrum *self,
                                     const char *name,
                                     const cpl_propertylist *plist,
                                     const char *key)
{
  cpl_ensure_code(self != NULL, CPL_ERROR_NULL_INPUT);

  assert(self->table != NULL);

  if (cpl_propertylist_has(plist, key)) {
    cpl_errorstate prestate = cpl_errorstate_get();
    const char *value = cpl_propertylist_get_string(plist, key);
    if (cpl_errorstate_is_equal(prestate)) {
      /* Prevent completely empty strings else cfitsio silently deletes the
       * keyword. */
      if (value != NULL && *value == '\0') {
        value = " ";
      }
      return cpl_table_set_column_unit(self->table, name, value);
    } else {
      return cpl_error_set_message(cpl_func, cpl_error_get_code(),
                "Could not set the unit for column '%s'. Likely the source '%s'"
                " keyword is not a string.", name, key);
    }
  } else {
    return cpl_error_set_message(cpl_func, CPL_ERROR_DATA_NOT_FOUND,
                "Could not set the unit for column '%s' since the '%s' keyword"
                " was not found.", name, key);
  }
}


const char *
irplib_sdp_spectrum_get_column_format(const irplib_sdp_spectrum *self,
                                      const char *name)
{
  cpl_ensure(self != NULL, CPL_ERROR_NULL_INPUT, NULL);
  assert(self->table != NULL);
  return cpl_table_get_column_format(self->table, name);
}


cpl_error_code
irplib_sdp_spectrum_set_column_format(irplib_sdp_spectrum *self,
                                      const char *name, const char *format)
{
  cpl_ensure_code(self != NULL, CPL_ERROR_NULL_INPUT);
  assert(self->table != NULL);
  return cpl_table_set_column_format(self->table, name, format);
}


static cpl_size
_irplib_sdp_spectrum_get_column_index(const irplib_sdp_spectrum *self,
                                      const char *name)
{
  cpl_size i, n;
  cpl_array *names;

  assert(self != NULL);
  assert(self->table != NULL);
  assert(name != NULL);

  /* Try find the index number of the column. */
  names = cpl_table_get_column_names(self->table);
  n = cpl_array_get_size(names);
  for (i = 0; i < n; ++i) {
    const char *namei = cpl_array_get_string(names, i);
    if (strcmp(namei, name) == 0) {
      cpl_array_delete(names);
      return i;
    }
  }
  cpl_array_delete(names);
  return (cpl_size)-1;
}


static const char *
_irplib_sdp_spectrum_get_column_keyword(const irplib_sdp_spectrum *self,
                                        const char *name, const char *keyword)
{
  cpl_size index;
  const char *result = NULL;

  assert(self != NULL);
  assert(self->proplist != NULL);
  assert(name != NULL);
  assert(keyword != NULL);

  index = _irplib_sdp_spectrum_get_column_index(self, name);
  if (index != (cpl_size)-1) {
    /* If the index number was found then try return the property value. */
    char *propname = cpl_sprintf("%s%"CPL_SIZE_FORMAT, keyword, index+1);
    if (cpl_propertylist_has(self->proplist, propname)) {
      result = cpl_propertylist_get_string(self->proplist, propname);
    }
    cpl_free(propname);
  } else {
    cpl_error_set_message(cpl_func, CPL_ERROR_DATA_NOT_FOUND,
                "Could not find '%s' keyword for column '%s'.", keyword, name);
  }
  return result;
}


static cpl_error_code
_irplib_sdp_spectrum_set_column_keyword(irplib_sdp_spectrum *self,
                                        const char *name,
                                        const char *value,
                                        const char *keyword,
                                        const char *comment)
{
  cpl_size index;
  char *propname, *pcomment;

  assert(self != NULL);
  assert(self->proplist != NULL);
  assert(name != NULL);
  assert(keyword != NULL);
  assert(comment != NULL);

  index = _irplib_sdp_spectrum_get_column_index(self, name);
  /* If the index was not found then return an error message. */
  if (index == (cpl_size)-1) {
    return cpl_error_set_message(cpl_func, CPL_ERROR_DATA_NOT_FOUND,
                "Could not find '%s' keyword for column '%s'.", keyword, name);
  }
  /* Since the index number was found then try set or add the property value. */
  cpl_error_code error = CPL_ERROR_NONE;
  propname = cpl_sprintf("%s%"CPL_SIZE_FORMAT, keyword, index+1);
  pcomment = cpl_sprintf("%s%"CPL_SIZE_FORMAT, comment, index+1);
  if (cpl_propertylist_has(self->proplist, propname)) {
    if (value != NULL) {
      error = cpl_propertylist_set_string(self->proplist, propname, value);
    } else {
      (void) cpl_propertylist_erase(self->proplist, propname);
    }
  } else if (value != NULL) {
    error = cpl_propertylist_append_string(self->proplist, propname, value);
    if (! error) {
      error = cpl_propertylist_set_comment(self->proplist, propname,
                                           pcomment);
      if (error) {
        /* Delete entry if we could not set the comment to maintain a
         * consistent state */
        cpl_errorstate prestate = cpl_errorstate_get();
        (void) cpl_propertylist_erase(self->proplist, propname);
        cpl_errorstate_set(prestate);
      }
    }
  }
  cpl_free(propname);
  cpl_free(pcomment);
  return error;
}


static void
_irplib_sdp_spectrum_erase_column_keywords(irplib_sdp_spectrum *self,
                                           const char *name)
{
  cpl_size index;

  assert(self != NULL);
  assert(self->proplist != NULL);
  assert(name != NULL);

  index = _irplib_sdp_spectrum_get_column_index(self, name);
  if (index != (cpl_size)-1) {
    char *propname = cpl_sprintf("%s%"CPL_SIZE_FORMAT, KEY_TUTYP, index+1);
    cpl_propertylist_erase(self->proplist, propname);
    cpl_free(propname);
    propname = cpl_sprintf("%s%"CPL_SIZE_FORMAT, KEY_TUCD, index+1);
    cpl_propertylist_erase(self->proplist, propname);
    cpl_free(propname);
    propname = cpl_sprintf("%s%"CPL_SIZE_FORMAT, KEY_TCOMM, index+1);
    cpl_propertylist_erase(self->proplist, propname);
    cpl_free(propname);
  }
}


const char *
irplib_sdp_spectrum_get_column_tutyp(const irplib_sdp_spectrum *self,
                                     const char *name)
{
  cpl_errorstate prestate = cpl_errorstate_get();
  const char *result;
  cpl_ensure(self != NULL && name != NULL, CPL_ERROR_NULL_INPUT, NULL);
  result = _irplib_sdp_spectrum_get_column_keyword(self, name, KEY_TUTYP);
  if (! cpl_errorstate_is_equal(prestate)) {
    cpl_error_set_where(cpl_func);
  }
  return result;
}


cpl_error_code
irplib_sdp_spectrum_set_column_tutyp(irplib_sdp_spectrum *self,
                                     const char *name, const char *tutyp)
{
  cpl_error_code error;
  cpl_ensure_code(self != NULL && name != NULL, CPL_ERROR_NULL_INPUT);
  error = _irplib_sdp_spectrum_set_column_keyword(self, name, tutyp,
                                                  KEY_TUTYP, KEY_TUTYP_COMMENT);
  if (error) {
    cpl_error_set_where(cpl_func);
  }
  return error;
}


cpl_error_code
irplib_sdp_spectrum_copy_column_tutyp(irplib_sdp_spectrum *self,
                                      const char *name,
                                      const cpl_propertylist *plist,
                                      const char *key)
{
  cpl_ensure_code(self != NULL, CPL_ERROR_NULL_INPUT);

  assert(self->table != NULL);

  if (cpl_propertylist_has(plist, key)) {
    cpl_errorstate prestate = cpl_errorstate_get();
    const char *value = cpl_propertylist_get_string(plist, key);
    if (cpl_errorstate_is_equal(prestate)) {
      return irplib_sdp_spectrum_set_column_tutyp(self, name, value);
    } else {
      cpl_size index = _irplib_sdp_spectrum_get_column_index(self, name) + 1;
      return cpl_error_set_message(cpl_func, cpl_error_get_code(),
                "Could not set '%s%"CPL_SIZE_FORMAT"' for column '%s'. Likely"
                " the source '%s' keyword is not a string.",
                KEY_TUTYP, index, name, key);
    }
  } else {
    cpl_size index = _irplib_sdp_spectrum_get_column_index(self, name) + 1;
    return cpl_error_set_message(cpl_func, CPL_ERROR_DATA_NOT_FOUND,
                "Could not set '%s%"CPL_SIZE_FORMAT"' for column '%s' since the"
                " '%s' keyword was not found.", KEY_TUTYP, index, name, key);
  }
}


const char *
irplib_sdp_spectrum_get_column_tucd(const irplib_sdp_spectrum *self,
                                    const char *name)
{
  cpl_errorstate prestate = cpl_errorstate_get();
  const char *result;
  cpl_ensure(self != NULL && name != NULL, CPL_ERROR_NULL_INPUT, NULL);
  result = _irplib_sdp_spectrum_get_column_keyword(self, name, KEY_TUCD);
  if (! cpl_errorstate_is_equal(prestate)) {
    cpl_error_set_where(cpl_func);
  }
  return result;
}


cpl_error_code
irplib_sdp_spectrum_set_column_tucd(irplib_sdp_spectrum *self,
                                    const char *name, const char *tucd)
{
  cpl_error_code error;
  cpl_ensure_code(self != NULL && name != NULL, CPL_ERROR_NULL_INPUT);
  error = _irplib_sdp_spectrum_set_column_keyword(self, name, tucd,
                                                  KEY_TUCD, KEY_TUCD_COMMENT);
  if (error) {
    cpl_error_set_where(cpl_func);
  }
  return error;
}


cpl_error_code
irplib_sdp_spectrum_copy_column_tucd(irplib_sdp_spectrum *self,
                                     const char *name,
                                     const cpl_propertylist *plist,
                                     const char *key)
{
  cpl_ensure_code(self != NULL, CPL_ERROR_NULL_INPUT);

  assert(self->table != NULL);

  if (cpl_propertylist_has(plist, key)) {
    cpl_errorstate prestate = cpl_errorstate_get();
    const char *value = cpl_propertylist_get_string(plist, key);
    if (cpl_errorstate_is_equal(prestate)) {
      return irplib_sdp_spectrum_set_column_tucd(self, name, value);
    } else {
      cpl_size index = _irplib_sdp_spectrum_get_column_index(self, name) + 1;
      return cpl_error_set_message(cpl_func, cpl_error_get_code(),
                "Could not set '%s%"CPL_SIZE_FORMAT"' for column '%s'. Likely"
                " the source '%s' keyword is not a string.",
                KEY_TUCD, index, name, key);
    }
  } else {
    cpl_size index = _irplib_sdp_spectrum_get_column_index(self, name) + 1;
    return cpl_error_set_message(cpl_func, CPL_ERROR_DATA_NOT_FOUND,
                "Could not set '%s%"CPL_SIZE_FORMAT"' for column '%s' since the"
                " '%s' keyword was not found.", KEY_TUCD, index, name, key);
  }
}


const char *
irplib_sdp_spectrum_get_column_tcomm(const irplib_sdp_spectrum *self,
                                     const char *name)
{
  cpl_errorstate prestate = cpl_errorstate_get();
  const char *result;
  cpl_ensure(self != NULL && name != NULL, CPL_ERROR_NULL_INPUT, NULL);
  result = _irplib_sdp_spectrum_get_column_keyword(self, name, KEY_TCOMM);
  if (! cpl_errorstate_is_equal(prestate)) {
    cpl_error_set_where(cpl_func);
  }
  return result;
}


cpl_error_code
irplib_sdp_spectrum_set_column_tcomm(irplib_sdp_spectrum *self,
                                     const char *name, const char *tcomm)
{
  cpl_error_code error;
  cpl_ensure_code(self != NULL && name != NULL, CPL_ERROR_NULL_INPUT);
  error = _irplib_sdp_spectrum_set_column_keyword(self, name, tcomm,
                                                  KEY_TCOMM, KEY_TCOMM_COMMENT);
  if (error) {
    cpl_error_set_where(cpl_func);
  }
  return error;
}


cpl_error_code
irplib_sdp_spectrum_copy_column_tcomm(irplib_sdp_spectrum *self,
                                      const char *name,
                                      const cpl_propertylist *plist,
                                      const char *key)
{
  cpl_ensure_code(self != NULL, CPL_ERROR_NULL_INPUT);

  assert(self->table != NULL);

  if (cpl_propertylist_has(plist, key)) {
    cpl_errorstate prestate = cpl_errorstate_get();
    const char *value = cpl_propertylist_get_string(plist, key);
    if (cpl_errorstate_is_equal(prestate)) {
      return irplib_sdp_spectrum_set_column_tcomm(self, name, value);
    } else {
      cpl_size index = _irplib_sdp_spectrum_get_column_index(self, name) + 1;
      return cpl_error_set_message(cpl_func, cpl_error_get_code(),
                "Could not set '%s%"CPL_SIZE_FORMAT"' for column '%s'. Likely"
                " the source '%s' keyword is not a string.",
                KEY_TCOMM, index, name, key);
    }
  } else {
    cpl_size index = _irplib_sdp_spectrum_get_column_index(self, name) + 1;
    return cpl_error_set_message(cpl_func, CPL_ERROR_DATA_NOT_FOUND,
                "Could not set '%s%"CPL_SIZE_FORMAT"' for column '%s' since the"
                " '%s' keyword was not found.", KEY_TCOMM, index, name, key);
  }
}


/*----------------------------------------------------------------------------*/
/**
 * @brief Replace the comment of a column description keyword.
 * @param self    The spectrum object to update.
 * @param name    The name of the column to update.
 * @param keyword The name of the keyword to update.
 * @param comment The string to use as keyword comment.
 * @return @c CPL_ERROR_NONE on success or an appropriate error code otherwise.
 *
 * This function will replace the comment of the keyword @c keyword of
 * the column @c name with the string @c comment. The string @c keyword
 * is the keyword name without the column index appended. The latter is
 * deduced from the column name @name. The new comment @c comment must not
 * be @a NULL.
 * If any error occurs then a error code is set and returned, otherwise
 * @c CPL_ERROR_NONE is returned on success.
 */
/*----------------------------------------------------------------------------*/

cpl_error_code
irplib_sdp_spectrum_replace_column_comment(irplib_sdp_spectrum *self,
                                           const char *name,
                                           const char *keyword,
                                           const char *comment)
{
    cpl_ensure_code((self != NULL), CPL_ERROR_NULL_INPUT);
    cpl_ensure_code((self->proplist != NULL), CPL_ERROR_ILLEGAL_INPUT);
    cpl_ensure_code((name != NULL) && (keyword != NULL) && (comment != NULL),
                    CPL_ERROR_NULL_INPUT);

    cpl_size index = _irplib_sdp_spectrum_get_column_index(self, name);
    if (index == (cpl_size)-1) {
      return cpl_error_set_message(cpl_func, CPL_ERROR_DATA_NOT_FOUND,
                  "Could not find column '%s'.", name);
    }

    char *propname = cpl_sprintf("%s%"CPL_SIZE_FORMAT, keyword, index + 1);
    if (!cpl_propertylist_has(self->proplist, propname)) {
        cpl_free(propname);
        return cpl_error_set_message(cpl_func, CPL_ERROR_DATA_NOT_FOUND,
                  "Could not find '%s' keyword for column '%s'.", keyword,
                  name);
    }

    cpl_propertylist_set_comment(self->proplist, propname, comment);
    cpl_free(propname);

    return CPL_ERROR_NONE;
}


const cpl_array *
irplib_sdp_spectrum_get_column_data(const irplib_sdp_spectrum *self,
                                    const char *name)
{
  cpl_ensure(self != NULL, CPL_ERROR_NULL_INPUT, NULL);
  assert(self->table != NULL);
  return cpl_table_get_array(self->table, name, 0);
}


cpl_error_code
irplib_sdp_spectrum_set_column_data(irplib_sdp_spectrum *self,
                                    const char *name, const cpl_array *array)
{
  cpl_ensure_code(self != NULL, CPL_ERROR_NULL_INPUT);
  assert(self->table != NULL);
  return cpl_table_set_array(self->table, name, 0, array);
}


static char * _irplib_make_regexp(const cpl_propertylist *plist,
                                  const char *extra)
{
  /* Minimum number of characters required for possible "^(", "|" or ")$"
   * fragments and a null character to end the string. */
  static const cpl_size min_chars_required = 6;

  /* Start, end and join fragments for the regular expression, to form a string
   * of the following form: "^(KEY1|KEY2 .. |KEYN)$" */
  static const char *start_fragment = "^(";
  static const char *end_fragment = ")$";
  static const char *join_fragment = "|";

  cpl_size extra_length = (extra != NULL ? (cpl_size) strlen(extra) : 0);
  cpl_size regexp_size, bytesleft, nkeys, i;
  char *writepos;
  char *regexp = NULL;

  assert(plist != NULL);

  nkeys = cpl_propertylist_get_size(plist);
  if (nkeys == 0) {
    /* Handle special case where plist is empty. */
    if (extra != NULL) {
      return cpl_sprintf("%s%s%s", start_fragment, extra, end_fragment);
    } else {
      return cpl_strdup("");
    }
  }

  /* Allocate enough space to store the regexp. 80 = FITS card width. */
  regexp_size = nkeys * 80 + min_chars_required + extra_length;
  regexp = cpl_malloc(regexp_size);

  bytesleft = regexp_size;
  writepos = regexp;
  for (i = 0; i < nkeys; ++i) {
    cpl_size name_length, fragment_length;
    const char *name, *fragment;

    /* Fetch the property name string. */
    const cpl_property *p = cpl_propertylist_get_const(plist, i);
    cpl_error_ensure(p != NULL, cpl_error_get_code(), goto cleanup,
      "Unexpected error accessing property structure %"CPL_SIZE_FORMAT".", i);
    name = cpl_property_get_name(p);
    cpl_error_ensure(name != NULL, cpl_error_get_code(), goto cleanup,
      "Unexpected error accessing the name of property %"CPL_SIZE_FORMAT".", i);
    name_length = (cpl_size) strlen(name);

    /* Figure out the regexp start/join string fragment to use. */
    fragment = (i == 0) ? start_fragment : join_fragment;
    fragment_length = (cpl_size) strlen(fragment);

    while (bytesleft <
           fragment_length + name_length + extra_length + min_chars_required)
    {
      /* Allocate more space if we still run out of space for the regexp.
       * Note: the realloc either succeeds or aborts on failure. */
      bytesleft += regexp_size;
      regexp_size += regexp_size;
      regexp = cpl_realloc(regexp, regexp_size);
      writepos = regexp + (regexp_size - bytesleft);
    }

    /* Write the start/join fragment and then the key name strings. */
    strncpy(writepos, fragment, bytesleft);
    bytesleft -= fragment_length;
    writepos += fragment_length;
    strncpy(writepos, name, bytesleft);
    bytesleft -= name_length;
    writepos += name_length;
  }

  /* Write the extra string and end fragment string to complete the regexp. */
  if (extra != NULL) {
    strncpy(writepos, join_fragment, bytesleft);
    bytesleft -= (cpl_size) strlen(join_fragment);
    writepos += (cpl_size) strlen(join_fragment);
    strncpy(writepos, extra, bytesleft);
    bytesleft -= extra_length;
    writepos += extra_length;
  }
  strncpy(writepos, end_fragment, bytesleft);
  /* Null terminate the string buffer for safety. */
  regexp[regexp_size-1] = '\0';

  return regexp;

cleanup:
  /* Cleanup in case of error: */
  cpl_free(regexp);
  return NULL;
}


irplib_sdp_spectrum * irplib_sdp_spectrum_load(const char *filename)
{
  cpl_error_code error;
  irplib_sdp_spectrum *obj;
  cpl_propertylist *plist = NULL;
  cpl_propertylist *tmpplist = NULL;
  cpl_table *table = NULL;
  cpl_array *names = NULL;
  cpl_array *emptyarray = NULL;
  cpl_size nelem, ext, i;
  char *regexp = NULL;

  cpl_ensure(filename != NULL, CPL_ERROR_NULL_INPUT, NULL);

  /* Load the property list from file, making sure the properties from the
   * primary HDU take precedence over those from the extension if any keywords
   * are duplicated. Note, we only load keywords known to the spectrum class. */
  plist = cpl_propertylist_load_regexp(filename, 0, ALL_KEYS_REGEXP, 0);
  cpl_error_ensure(plist != NULL, cpl_error_get_code(), goto cleanup,
      "Could not load property list from primary HDU when loading file '%s'.",
      filename);

  /* We have to create a regexp to filter out keywords already loaded from the
   * primary HDU. */
  regexp = _irplib_make_regexp(plist, NULL);
  cpl_error_ensure(regexp != NULL, cpl_error_get_code(), goto cleanup,
                   "Could not create regular expression to filter keywords.");

  /* Try find the spectrum extension from which to load the table. If the
   * extension name cannot be found then just use the first extension. */
  ext = cpl_fits_find_extension(filename, KEY_EXTNAME_VALUE);
  cpl_error_ensure(ext != (cpl_size)-1, cpl_error_get_code(), goto cleanup,
                   "Failed to get the extension '%s' from file '%s'.",
                   KEY_EXTNAME_VALUE, filename);
  if (ext == 0) ext = 1;

  /* Load only the SDP keywords from the extension. */
  tmpplist = cpl_propertylist_load_regexp(filename, ext, ALL_KEYS_REGEXP, 0);
  cpl_error_ensure(tmpplist != NULL, cpl_error_get_code(), goto cleanup,
                   "Could not load property list from extension %"
                   CPL_SIZE_FORMAT" when loading file '%s'.", ext, filename);

  /* Append keywords to plist that are not already in plist. */
  error = cpl_propertylist_copy_property_regexp(plist, tmpplist, regexp, 1);
  cpl_error_ensure(! error, error, goto cleanup,
                   "Failed to append keywords from file '%s' extension %"
                   CPL_SIZE_FORMAT".", filename, ext);

  /* Delete temporary objects that are no longer needed. */
  cpl_propertylist_delete(tmpplist);
  tmpplist = NULL;
  cpl_free(regexp);
  regexp = NULL;

  table = cpl_table_load(filename, (int)ext, CPL_TRUE);
  cpl_error_ensure(table != NULL, cpl_error_get_code(), goto cleanup,
                   "Could not load the spectrum table from extension %"
                   CPL_SIZE_FORMAT" when loading file '%s'.", ext, filename);

  /* Set the nelem value from the NELEM keyword if found, else work it out. */
  if (cpl_propertylist_has(plist, KEY_NELEM)) {
    cpl_errorstate prestate = cpl_errorstate_get();
    nelem = (cpl_size) cpl_propertylist_get_long_long(plist, KEY_NELEM);
    /* Remove NELEM since the value is instead stored in the nelem variable. */
    cpl_propertylist_erase(plist, KEY_NELEM);
    cpl_error_ensure(cpl_errorstate_is_equal(prestate), cpl_error_get_code(),
                  goto cleanup, "Could not process the temporary '%s' keyword.",
                  KEY_NELEM);
  } else {
    cpl_msg_warning(cpl_func,
                    "Keyword '%s' not found in file '%s'. Possibly corrupted."
                    " Will try find correct value from the table and continue.",
                    KEY_NELEM, filename);
    nelem = 0;
    if (cpl_table_get_nrow(table) > 0) {
      names = cpl_table_get_column_names(table);
      if (names != NULL) {
        if (cpl_array_get_size(names) > 0) {
          const char *name = cpl_array_get_string(names, 0);
          nelem = cpl_table_get_column_depth(table, name);
        }
        cpl_array_delete(names);
        names = NULL;
      }
    }
  }

  names = cpl_table_get_column_names(table);
  cpl_error_ensure(names != NULL, cpl_error_get_code(), goto cleanup,
          "Could not get table column names when loading file '%s'.", filename);
  for (i = 0; i < cpl_array_get_size(names); ++i) {
    int j;
    const char *name = cpl_array_get_string(names, 0);
    cpl_type type = cpl_table_get_column_type(table, name);
    if ((type & CPL_TYPE_POINTER) == 0) continue;  /* Only handle array columns.*/
    for (j = 0; j < cpl_table_get_nrow(table); ++j) {
      if (cpl_table_get_array(table, name, j) != NULL) continue;
      emptyarray = cpl_array_new(nelem, type & (~CPL_TYPE_POINTER));
      cpl_error_ensure(emptyarray != NULL, cpl_error_get_code(), goto cleanup,
            "Could not create empty array when spectrum table from file '%s'.",
            filename);
      error = cpl_table_set_array(table, name, j, emptyarray);
      cpl_array_delete(emptyarray);
      emptyarray = NULL;
    }
  }
  cpl_array_delete(names);

  /* Create new spectrum instance and return it. */
  obj = cpl_malloc(sizeof(irplib_sdp_spectrum));
  obj->nelem = nelem;
  obj->proplist = plist;
  obj->table = table;
  return obj;

cleanup:
  /* Perform memory cleanup if an error occurred. The cpl_error_ensure macros
   * will send the control flow to this point when an error is detected.
   * Note: cpl_*_delete functions already check for NULL pointers. */
  cpl_propertylist_delete(plist);
  cpl_propertylist_delete(tmpplist);
  cpl_table_delete(table);
  cpl_array_delete(names);
  cpl_array_delete(emptyarray);
  cpl_free(regexp);
  return NULL;
}


cpl_error_code irplib_sdp_spectrum_save(const irplib_sdp_spectrum *self,
                                        const char *filename,
                                        const cpl_propertylist *extra_pheader,
                                        const cpl_propertylist *extra_theader)
{
  cpl_error_code error;
  cpl_propertylist *primarykeys = NULL;
  cpl_propertylist *tablekeys = NULL;
  char *regexp = NULL;

  cpl_ensure_code(self != NULL, CPL_ERROR_NULL_INPUT);

  assert(self->proplist != NULL);
  assert(self->table != NULL);

  /* Make a regular expression to filter out all keywords found in the spectrum
   * object's proplist and NELEM from the extra header keywords. */
  regexp = _irplib_make_regexp(self->proplist, KEY_NELEM);
  cpl_error_ensure(regexp != NULL, cpl_error_get_code(), goto cleanup,
                   "Could not create regular expression to filter keywords.");

  /* Copy out keywords that should be in the primary HDU header from the full
   * list of keywords in proplist. */
  primarykeys = cpl_propertylist_new();
  error = cpl_propertylist_copy_property_regexp(primarykeys, self->proplist,
                                                PRIMARY_HDU_KEYS_REGEXP, 0);
  cpl_error_ensure(! error, error, goto cleanup,
                   "Failed to extract keywords for primary HDU.");

  /* Use a different comment name for OBJECT in the primary HDU to more closely
   * follow standard document. */
  if (cpl_propertylist_has(primarykeys, KEY_OBJECT)) {
    error = cpl_propertylist_set_comment(primarykeys, KEY_OBJECT,
                                         KEY_OBJECT_PHDU_COMMENT);
    cpl_error_ensure(! error, error, goto cleanup,
              "Could not update comment for '%s' in primary HDU.", KEY_OBJECT);
  }

  /* Copy any extra keywords that are not already in the primary HDU header. */
  if (extra_pheader != NULL) {
    error = cpl_propertylist_copy_property_regexp(primarykeys, extra_pheader,
                                                  regexp, 1);
    cpl_error_ensure(! error, error, goto cleanup,
                     "Could not add extra keywords for primary HDU.");
  }

  /* Copy out keywords for the table header from all in proplist. */
  tablekeys = cpl_propertylist_new();
  error = cpl_propertylist_copy_property_regexp(tablekeys, self->proplist,
                                                EXTENSION_HDU_KEYS_REGEXP, 0);
  cpl_error_ensure(! error, error, goto cleanup,
                   "Failed to extract keywords for extension HDU.");

  /* Add the NELEM keyword from the nelem variable. */
  cpl_error_ensure(self->nelem <= INT_MAX, CPL_ERROR_INCOMPATIBLE_INPUT,
                   goto cleanup,
                   "The value for the keyword '%s' is too big (> %d).",
                   KEY_NELEM, INT_MAX);
  error = cpl_propertylist_append_int(tablekeys, KEY_NELEM,
                                      (int) self->nelem);
  error |= cpl_propertylist_set_comment(tablekeys, KEY_NELEM,
                                        KEY_NELEM_COMMENT);
  cpl_error_ensure(! error, error, goto cleanup,
              "Could not add keyword '%s' to primary HDU or set the comment.",
              KEY_NELEM);

  /* Copy extra keywords that are not already in the extension HDU header. */
  if (extra_theader != NULL) {
    error = cpl_propertylist_copy_property_regexp(tablekeys, extra_theader,
                                                  regexp, 1);
    cpl_error_ensure(! error, error, goto cleanup,
                     "Could not add extra keywords for extension HDU.");
  }

  cpl_free(regexp);
  regexp = NULL;

  /* Add some mandatory keywords with default values that are still not found
   * in the primary or extension property lists, since they were not set in the
   * spectrum or in the extra header lists. */
  error = CPL_ERROR_NONE;
  if (! cpl_propertylist_has(primarykeys, KEY_ORIGIN)) {
    error |= cpl_propertylist_append_string(primarykeys, KEY_ORIGIN,
                                            KEY_ORIGIN_VALUE);
    error |= cpl_propertylist_set_comment(primarykeys, KEY_ORIGIN,
                                          KEY_ORIGIN_COMMENT);
  }
  if (! cpl_propertylist_has(primarykeys, KEY_PRODLVL)) {
    error |= cpl_propertylist_append_int(primarykeys, KEY_PRODLVL,
                                         KEY_PRODLVL_VALUE);
    error |= cpl_propertylist_set_comment(primarykeys, KEY_PRODLVL,
                                          KEY_PRODLVL_COMMENT);
  }
  if (! cpl_propertylist_has(primarykeys, KEY_SPECSYS)) {
    error |= cpl_propertylist_append_string(primarykeys, KEY_SPECSYS,
                                            KEY_SPECSYS_VALUE);
    error |= cpl_propertylist_set_comment(primarykeys, KEY_SPECSYS,
                                          KEY_SPECSYS_COMMENT);
  }
  if (! cpl_propertylist_has(primarykeys, KEY_FLUXERR)) {
    error |= cpl_propertylist_append_int(primarykeys, KEY_FLUXERR,
                                         KEY_FLUXERR_VALUE);
    error |= cpl_propertylist_set_comment(primarykeys, KEY_FLUXERR,
                                          KEY_FLUXERR_COMMENT);
  }
  if (! cpl_propertylist_has(tablekeys, KEY_VOCLASS)) {
    error |= cpl_propertylist_append_string(tablekeys, KEY_VOCLASS,
                                            KEY_VOCLASS_VALUE);
    error |= cpl_propertylist_set_comment(tablekeys, KEY_VOCLASS,
                                          KEY_VOCLASS_COMMENT);
  }
  if (! cpl_propertylist_has(tablekeys, KEY_VOPUB)) {
    error |= cpl_propertylist_append_string(tablekeys, KEY_VOPUB,
                                            KEY_VOPUB_VALUE);
    error |= cpl_propertylist_set_comment(tablekeys, KEY_VOPUB,
                                          KEY_VOPUB_COMMENT);
  }
  if (! cpl_propertylist_has(tablekeys, KEY_EXTNAME)) {
    error |= cpl_propertylist_append_string(tablekeys, KEY_EXTNAME,
                                            KEY_EXTNAME_VALUE);
    error |= cpl_propertylist_set_comment(tablekeys, KEY_EXTNAME,
                                          KEY_EXTNAME_COMMENT);
  }
  if (! cpl_propertylist_has(tablekeys, KEY_INHERIT)) {
    error |= cpl_propertylist_append_bool(tablekeys, KEY_INHERIT,
                                          KEY_INHERIT_VALUE);
    error |= cpl_propertylist_set_comment(tablekeys, KEY_INHERIT,
                                          KEY_INHERIT_COMMENT);
  }
  cpl_error_ensure(! error, cpl_error_get_code(), goto cleanup,
                   "Could not set default header keywords for file '%s'.",
                   filename);

  error = cpl_table_save(self->table, primarykeys, tablekeys, filename,
                         CPL_IO_CREATE);
  cpl_error_ensure(! error, error, goto cleanup,
                   "Could not save the spectrum table to file '%s'.", filename);

  cpl_propertylist_delete(primarykeys);
  cpl_propertylist_delete(tablekeys);

  return CPL_ERROR_NONE;

cleanup:
  /* Cleanup memory if an error occurred. Note: cpl_*_delete functions already
   * check for NULL pointers. */
  cpl_propertylist_delete(primarykeys);
  cpl_propertylist_delete(tablekeys);
  cpl_free(regexp);
  return cpl_error_get_code();
}


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
                                        const char                * filename)
{
  const char       * procat;
  cpl_propertylist * plist = NULL;
  cpl_frame        * product_frame = NULL;
  cpl_error_code     error;

  cpl_ensure_code(allframes  != NULL, CPL_ERROR_NULL_INPUT);
  cpl_ensure_code(parlist    != NULL, CPL_ERROR_NULL_INPUT);
  cpl_ensure_code(usedframes != NULL, CPL_ERROR_NULL_INPUT);
  cpl_ensure_code(spectrum   != NULL, CPL_ERROR_NULL_INPUT);
  cpl_ensure_code(recipe     != NULL, CPL_ERROR_NULL_INPUT);
  cpl_ensure_code(applist    != NULL, CPL_ERROR_NULL_INPUT);
  cpl_ensure_code(pipe_id    != NULL, CPL_ERROR_NULL_INPUT);
  cpl_ensure_code(dict_id    != NULL, CPL_ERROR_NULL_INPUT);
  cpl_ensure_code(filename   != NULL, CPL_ERROR_NULL_INPUT);

  procat = cpl_propertylist_get_string(applist, CPL_DFS_PRO_CATG);
  cpl_error_ensure(procat != NULL, cpl_error_get_code(), goto cleanup,
              "Could not find keyword '%s' in 'applist'.", CPL_DFS_PRO_CATG);

  /* Create product frame */
  product_frame = cpl_frame_new();
  error = cpl_frame_set_filename(product_frame, filename);
  error |= cpl_frame_set_tag(product_frame, procat);
  error |= cpl_frame_set_type(product_frame, CPL_FRAME_TYPE_TABLE);
  error |= cpl_frame_set_group(product_frame, CPL_FRAME_GROUP_PRODUCT);
  error |= cpl_frame_set_level(product_frame, CPL_FRAME_LEVEL_FINAL);
  cpl_error_ensure(! error, cpl_error_get_code(), goto cleanup,
                   "Failed to setup the product frame.");

  /* Check if we should return the header information actually filled or just
   * create a temporary local list. */
  if (header != NULL) {
    cpl_propertylist_empty(header);
    plist = header;
  } else {
    plist = cpl_propertylist_new();
  }

  /* Add any QC parameters here. */
  error = cpl_propertylist_append(plist, applist);
  cpl_error_ensure(! error, error, goto cleanup,
          "Could not append extra keywords when writing file '%s'.", filename);

  /* Add DataFlow keywords. */
  error = cpl_dfs_setup_product_header(plist, product_frame, usedframes,
                                       parlist, recipe, pipe_id, dict_id,
                                       inherit);
  cpl_error_ensure(! error, error, goto cleanup,
          "Failed to setup DFS keywords when writing file '%s'.", filename);

  /* We have to update the extra keywords again for the primary HDU to make
   * sure we have the ability to override what cpl_dfs_setup_product_header
   * sets. The reason for still having the cpl_propertylist_append above is to
   * make sure we use comments as given by the applist and not as found in the
   * raw file we inherit from. The SDP format prefers standardised comments, not
   * necessarily used by the raw files. */
  error = cpl_propertylist_copy_property_regexp(plist, applist, ".*", 0);
  cpl_error_ensure(! error, error, goto cleanup,
          "Could not update extra keywords when writing file '%s'.", filename);

  if (remregexp != NULL) {
    cpl_errorstate prestate = cpl_errorstate_get();
    (void) cpl_propertylist_erase_regexp(plist, remregexp, 0);
    cpl_error_ensure(cpl_errorstate_is_equal(prestate), cpl_error_get_code(),
                     goto cleanup,
                     "Failed to filter keywords when writing file '%s'.",
                     filename);
  }

  error = irplib_sdp_spectrum_save(spectrum, filename, plist, tablelist);
  cpl_error_ensure(! error, error, goto cleanup,
                   "Failed to save SPD spectrum to file '%s'.", filename);

  /* Optionally return the SDP keywords that were written to the output. */
  if (header != NULL) {
    error = cpl_propertylist_copy_property_regexp(header, spectrum->proplist,
                                                  ".*", 0);
    cpl_error_ensure(! error, error, goto cleanup,
                     "Could not return SDP keywords in header output.");
  }

  /* Insert the frame of the saved file in the input frameset. */
  error = cpl_frameset_insert(allframes, product_frame);
  cpl_error_ensure(! error, error, goto cleanup,
        "Failed to insert new product frame when writing file '%s'.", filename);

  /* Delete output property list if it was only a temporary local object. */
  if (plist != header) cpl_propertylist_delete(plist);

  return CPL_ERROR_NONE;

cleanup:
  /* If an error occurred we come here to cleanup memory. Note that the delete
   * functions already check for NULL pointers. */
  if (header != NULL) {
    cpl_errorstate prestate = cpl_errorstate_get();
    (void) cpl_propertylist_empty(header);
    cpl_errorstate_set(prestate);
  } else {
    cpl_propertylist_delete(plist);
  }
  cpl_frame_delete(product_frame);
  return cpl_error_get_code();
}


void irplib_sdp_spectrum_dump(const irplib_sdp_spectrum *self, FILE *stream)
{
  if (stream == NULL) {
    stream = stdout;
  }
  if (self == NULL) {
    fprintf(stream, "NULL SDP spectrum\n\n");
    return;
  }

  assert(self->proplist != NULL);
  assert(self->table != NULL);

  fprintf(stream, "SDP spectrum at address %p\n", (void*)self);
  fprintf(stream, "NELEM = %"CPL_SIZE_FORMAT"\n", self->nelem);
  cpl_propertylist_dump(self->proplist, stream);
  cpl_table_dump_structure(self->table, stream);
  cpl_table_dump(self->table, 0, cpl_table_get_nrow(self->table), stream);
}


#ifdef IRPLIB_USE_FITS_UPDATE_CHECKSUM

/**
 * @brief Updates the FITS standard CHECKSUM and DATASUM keywords.
 * @param filename  The name of the FITS file to update.
 * @return a CPL error code indicating success or failure.
 *
 * @note To use this function one needs to declare the preprocessor macro
 *   @c IRPLIB_USE_FITS_UPDATE_CHECKSUM when compiling the @c libirplib.a
 *   library or including the header files. Also, additional linker flags to
 *   the cfitsio library need to be added when any binaries are linked with
 *   @c libirplib.a.
 */
cpl_error_code irplib_fits_update_checksums(const char* filename)
{
  fitsfile* filehandle;
  int error = 0; /* must be initialised to zero before call to cfitsio. */

  if (fits_open_diskfile(&filehandle, filename, READWRITE, &error)) {
    return cpl_error_set_message(cpl_func, CPL_ERROR_FILE_IO,
                "Could not open file '%s' to update CHECKSUM keywords"
                " (error = %d).", filename, error);
  }

  int i = 0;
  while (! fits_movabs_hdu(filehandle, ++i, NULL, &error)) {
    if (fits_write_chksum(filehandle, &error)) {
      return cpl_error_set_message(cpl_func, CPL_ERROR_FILE_IO,
                  "Could not update the CHECKSUM keywords in '%s' HDU %d"
                  " (error = %d).", filename, i, error);
    }
  }
  /* Reset after normal error */
  if (error == END_OF_FILE) error = 0;

  if (fits_close_file(filehandle, &error)) {
    return cpl_error_set_message(cpl_func, CPL_ERROR_FILE_IO,
                "There was a problem trying to close the file '%s'"
                " (error = %d).", filename, error);
  }
  return CPL_ERROR_NONE;
}

#endif /* IRPLIB_USE_FITS_UPDATE_CHECKSUM */
