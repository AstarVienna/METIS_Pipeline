/*
 * This file is part of the HDRL
 * Copyright (C) 2013 European Southern Observatory
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

#ifndef HDRL_OVERSCAN_DEFS_H
#define HDRL_OVERSCAN_DEFS_H

#ifndef HDRL_USE_PRIVATE
#error This file is not allowed to be included outside of hdrl
#endif

#include "hdrl_types.h"
#include "hdrl_image.h"
#include <cpl.h>

CPL_BEGIN_DECLS

/** @cond PRIVATE */

/*----------------------------------------------------------------------------*/
/**
  @struct _hdrl_overscan_compute_result_
  @brief Contains the Overscan Computation results
 */
/*----------------------------------------------------------------------------*/
struct _hdrl_overscan_compute_result_
{
    /** The direction in which the image were collapsed in the computation */
    hdrl_direction      correction_direction;     
    /** The overscan correction as a 1D double image */
    hdrl_image       *   correction;
    /** The number of good pixels that contributed as a 1D double image */
    cpl_image       *   contribution;
    /** The \f$\chi^{2}\f$ as a 1D double image */
    cpl_image       *   chi2;
    /** The reduced \f$\chi^{2}\f$ as a 1D double image */
    cpl_image       *   red_chi2;
    /** The low threshold below which the pixels are rejected 
        as a 1D double image. Only for sigma-clipping collapsing method */
    cpl_image       *   sigclip_reject_low;
    /** The high threshold above which the pixels are rejected 
        as a 1D double image. Only for sigma-clipping collapsing method */
    cpl_image       *   sigclip_reject_high;
};

/*----------------------------------------------------------------------------*/
/**
  @struct _hdrl_overscan_correct_result_
  @brief Contains the Overscan Correction results
 */
/*----------------------------------------------------------------------------*/
struct _hdrl_overscan_correct_result_
{
    /** The Overscan corrected image */
    hdrl_image   *   corrected;
    /** Pixels marked as bad by algorithm, encoded by user choice */
    cpl_image   *   badmask;
};

/** @endcond */

CPL_END_DECLS

#endif
