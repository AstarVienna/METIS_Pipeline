/*
 * This file is part of the IRPLIB Pipeline
 * Copyright (C) 2002,2003,2014 European Southern Observatory
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/

#include "irplib_fft.h"

/*----------------------------------------------------------------------------*/
/**
 * @defgroup irplib_fft     FFT functionality
 */
/*----------------------------------------------------------------------------*/

/**@{*/


/*----------------------------------------------------------------------------*/
/**
 * @internal
 * @brief Determine the shift between two images
 * @param self    The base image
 * @param other   The image that is shifted relative to the first one
 * @param px      The X-shift relative to the first image
 * @param py      The Y-shift relative to the first image
 * @return CPL_ERROR_NONE on success otherwise the relevant CPL error
 * @see cpl_fft_image_test_correlate(()
 *
 *
 */
/*----------------------------------------------------------------------------*/
cpl_error_code irplib_image_find_shift(const cpl_image * self,
                                       const cpl_image * other,
                                       double * px,
                                       double * py)
{
    const cpl_size  nx    = cpl_image_get_size_x(self);
    const cpl_size  ny    = cpl_image_get_size_y(self);
    const cpl_size  type  = cpl_image_get_type(self);
    const size_t    bufsz = (size_t)(nx * ny)
        * cpl_type_get_sizeof(type | CPL_TYPE_COMPLEX);
    cpl_imagelist * iml;
    cpl_imagelist * fml;
    cpl_image *     fself;
    cpl_image *     fother;
    void *          fdata;
    cpl_error_code  code = CPL_ERROR_NONE;

    cpl_ensure_code(px != NULL, CPL_ERROR_NULL_INPUT);
    cpl_ensure_code(py != NULL, CPL_ERROR_NULL_INPUT);

    iml = cpl_imagelist_new();
    /* Input images _not_ modified */
    CPL_DIAG_PRAGMA_PUSH_IGN(-Wcast-qual);
    cpl_imagelist_set(iml, (cpl_image *)self,  0);
    cpl_imagelist_set(iml, (cpl_image *)other, 1);
    CPL_DIAG_PRAGMA_POP;

    fdata = cpl_calloc(2, bufsz); /* Don't need two calloc()s here */

    fml = cpl_imagelist_new();
    fself  = cpl_image_wrap(nx, ny, type | CPL_TYPE_COMPLEX, fdata);
    fother = cpl_image_wrap(nx, ny, type | CPL_TYPE_COMPLEX, (char*)fdata
                            + bufsz);

    cpl_imagelist_set(fml, fself,  0);
    cpl_imagelist_set(fml, fother, 1);

    if (cpl_fft_imagelist(fml, iml, CPL_FFT_FORWARD)) {
        code = cpl_error_set_where(cpl_func);
    } else {
        /* Should not be able to fail now */
        cpl_size    xmax = 1, ymax = 1;
        /* Share pixel buffer with fself which is not needed at the same time */
        cpl_image * imgpos = cpl_image_wrap(nx, ny, type,
                                            cpl_image_get_data(fself));

        /* Cross-correlate */
        cpl_image_conjugate(fother, fother);
        cpl_image_multiply(fother, fself);

        cpl_fft_image(imgpos, fother, CPL_FFT_BACKWARD | CPL_FFT_NOSCALE);

        cpl_image_get_maxpos(imgpos, &xmax, &ymax);

        (void)cpl_image_unwrap(imgpos);

        /* The pixel position starts from 1, the offset from 0 */
        xmax--;
        ymax--;

        /* The offset is signed, from -N/2 to N/2-1 */
        *px = 2 * xmax >= nx ? xmax - nx : xmax;
        *py = 2 * ymax >= ny ? ymax - ny : ymax;
    }

    cpl_imagelist_unwrap(iml);
    cpl_image_unwrap(cpl_imagelist_unset(fml, 1));
    cpl_imagelist_delete(fml);

    return code;
}

/**@}*/
