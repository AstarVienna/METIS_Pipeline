/* $Id: irplib_wavecal.h,v 1.18 2012-08-03 21:05:33 llundin Exp $
 *
 * This file is part of the IRPLIB Pipeline
 * Copyright (C) 2002,2003 European Southern Observatory
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02111-1307  USA
 */

/*
 * $Author: llundin $
 * $Date: 2012-08-03 21:05:33 $
 * $Revision: 1.18 $
 * $Name: not supported by cvs2svn $
 */

#ifndef IRPLIB_WAVECAL_H
#define IRPLIB_WAVECAL_H

/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/

#include <cpl.h>


/*-----------------------------------------------------------------------------
                                   Define
 -----------------------------------------------------------------------------*/

#define IRPLIB_WAVECAL_MODEL_COEFFS       4
/* The number of columns is 5 + IRPLIB_WAVECAL_MODEL_COEFFS */
#define IRPLIB_WAVECAL_MODEL_COLS         9

#define IRPLIB_WAVECAL_LAB_MODE           "SpecMode"
#define IRPLIB_WAVECAL_LAB_RESID          "Residual"
#define IRPLIB_WAVECAL_LAB_ORDER          "Fit_Order"
#define IRPLIB_WAVECAL_LAB_XMIN           "XMin"
#define IRPLIB_WAVECAL_LAB_XMAX           "XMax"
#define IRPLIB_WAVECAL_LAB_C1             "C_1"
#define IRPLIB_WAVECAL_LAB_C2             "C_2"
#define IRPLIB_WAVECAL_LAB_C3             "C_3"
#define IRPLIB_WAVECAL_LAB_C4             "C_4"

#define IRPLIB_WAVECAL_LAB_WAVE           "WAVELENGTH"
#define IRPLIB_WAVECAL_LAB_INTENS         "INTENSITY"


/*-----------------------------------------------------------------------------
                               New Types
 -----------------------------------------------------------------------------*/

/* Any spectrum model must have these members first! */
typedef struct {
    cpl_size             cost;    /* May be incremented for cost counting */
    cpl_size             xcost;   /* Ditto (can exclude failed fills) */
    cpl_size             ulines;  /* May be set to number of lines used */

} irplib_base_spectrum_model;

typedef struct {
    cpl_size             cost;    /* May be incremented for cost counting */
    cpl_size             xcost;   /* Ditto (can exclude failed fills) */
    cpl_size             ulines;  /* May be set to number of lines used */

    double               wslit;  /* Slit Width */
    double               wfwhm;  /* FWHM of transfer function */
    double               xtrunc; /* Truncate transfer function beyond xtrunc,
                                    xtrunc > 0 */
    const cpl_bivector * lines;  /* Catalogue of intensities, with
                                    increasing X-vector elements */
    cpl_vector         * linepix;  /* Catalogue of line pixel positions
                                      - zero for uninitialized */
    cpl_vector         * erftmp;  /* Temporary storage for erf() values
                                      - zero for uninitialized */
} irplib_line_spectrum_model;

/*-----------------------------------------------------------------------------
                                   Function Prototypes
 -----------------------------------------------------------------------------*/

cpl_error_code
irplib_polynomial_find_1d_from_correlation_all(cpl_polynomial *,
                                               int,
                                               const cpl_vector *,
                                               int, int,
                                               irplib_base_spectrum_model *,
                                               cpl_error_code (*)
                                               (cpl_vector *,
                                                const cpl_polynomial *,
                                                irplib_base_spectrum_model *),
                                               double,
                                               double,
                                               int,
                                               int,
                                               int,
                                               int,
                                               cpl_boolean,
                                               double *);

cpl_error_code
irplib_bivector_find_shift_from_correlation(cpl_bivector *,
                                            const cpl_polynomial *,
                                            const cpl_vector *,
                                            irplib_base_spectrum_model *,
                                            cpl_error_code (*)
                                            (cpl_vector *,
                                             const cpl_polynomial *,
                                             irplib_base_spectrum_model *),
                                            int,
                                            cpl_boolean,
                                            double *);

cpl_error_code
irplib_polynomial_shift_1d_from_correlation(cpl_polynomial *,
                                            const cpl_vector *,
                                            irplib_base_spectrum_model *,
                                            cpl_error_code (*)
                                            (cpl_vector *,
                                             const cpl_polynomial *,
                                             irplib_base_spectrum_model *),
                                            int, cpl_boolean, double *);

cpl_error_code
irplib_polynomial_find_1d_from_correlation(cpl_polynomial *, int,
                                           const cpl_vector *,
                                           irplib_base_spectrum_model *,
                                           cpl_error_code (*)
                                           (cpl_vector *,
                                            const cpl_polynomial *,
                                            irplib_base_spectrum_model *),
                                           double, double,
                                           int, int, double *);

cpl_error_code irplib_vector_fill_line_spectrum(cpl_vector *,
                                                const cpl_polynomial *,
                                                irplib_base_spectrum_model *);

cpl_error_code irplib_vector_fill_logline_spectrum(cpl_vector *,
                                                   const cpl_polynomial *,
                                                   irplib_base_spectrum_model *);

cpl_error_code
irplib_vector_fill_line_spectrum_fast(cpl_vector *,
                                      const cpl_polynomial *,
                                      irplib_base_spectrum_model *);

cpl_error_code
irplib_vector_fill_logline_spectrum_fast(cpl_vector *,
                                         const cpl_polynomial *,
                                         irplib_base_spectrum_model *);

cpl_error_code irplib_plot_spectrum_and_model(const cpl_vector *,
                                              const cpl_polynomial *,
                                              irplib_base_spectrum_model *,
                                              cpl_error_code (*)
                                              (cpl_vector *,
                                               const cpl_polynomial *,
                                               irplib_base_spectrum_model *));

cpl_error_code irplib_polynomial_fit_2d_dispersion(cpl_polynomial *,
                                                   const cpl_image *,
                                                   int, double *);

int irplib_bivector_count_positive(const cpl_bivector *, double, double);

#endif
