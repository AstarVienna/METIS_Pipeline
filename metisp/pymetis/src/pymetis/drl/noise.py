"""
This file is part of the METIS Pipeline.
Copyright (C) 2024 European Southern Observatory

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
"""

import math

import cpl
import hdrl
from cpl.core import (Image as CplImage,
                      ImageList as CplImageList,
                      Mask as CplMask,
                      Msg)
from hdrl.core import Image as HdrlImage, ImageList as HdrlImageList


from pymetis.drl.image import zeros_like


def estimate_noise(
        image: CplImage,
        read_noise: float
) -> HdrlImage:
    """
    Turn a raw CPL image into an HDRL image with basic noise estimates.
    """

    Msg.info("estimate_noise",
             f"Estimating noise for raw image, readnoise = {read_noise}")

    noise = zeros_like(image)
    noise.copy_into(image, 0, 0)

    # add read noise plus shot noise
    noise.add_scalar(read_noise ** 2)
    noise.power(0.5)

    return HdrlImage(image, noise)


def estimate_noise_list(
        images: CplImageList,
        read_noise: float,
) -> HdrlImageList:
    """
    Routine to turn a cpl Imagelist of raw images into a hdrl ImageList with
    basic noise estimates.
    """
    images_hdrl = HdrlImageList()

    for im in images:
        image_hdrl = estimate_noise(im, read_noise)
        images_hdrl.append(image_hdrl)

    return images_hdrl


def calculate_outliers(
        image: HdrlImage,
        *,
        kappa_low: float,
        kappa_high: float,
) -> tuple[CplMask, CplMask]:
    """
    Calculate masks for outlier pixels, with kappa-sigma clipping, using HDRL BPM functions.
    """
    Msg.info("calculate_outliers",
             f"Identifying outlier pixels ({kappa_low=}, {kappa_high=})")

    # Flag pixels deviating from their median-filtered neighbourhood by more than kappa
    # sigma. Hot and cold pixels are separated by running the filter twice with the
    # other side's kappa infinite -- a finite stand-in (say 1000) would flag any
    # sufficiently extreme pixel on both sides. The border must be filtered too:
    # `Border.NOP` leaves the smoothed image zero there, so every border pixel comes
    # out as a "hot" outlier of its own full value.
    # TODO: confirm/explore best input parameters based on realistic data
    max_iter = 6
    filter_x = 5
    filter_y = 5

    def flag(low: float, high: float) -> CplMask:
        return hdrl.func.BPM2D.Filter(low, high, max_iter, cpl.core.Filter.MEDIAN, cpl.core.Border.FILTER,
                                      filter_x, filter_y).compute(image)

    mask_hot = flag(math.inf, kappa_high)
    mask_cold = flag(kappa_low, math.inf)

    return mask_hot, mask_cold
