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

import cpl
import hdrl
from cpl.core import (Image as CplImage,
                      ImageList as CplImageList,
                      Mask as CplMask,
                      Msg)
from hdrl.core import Image as HdrlImage, ImageList as HdrlImageList

from typing import Literal, Optional

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

    # get masks from thresholds for bad, hot and cold pixels
    # count the number of bad pixels in each, for later, then
    # change to Image type from mask for later calculations

    image_median = image.get_median()
    image_rms = image.get_stdev()

    # user HDRL function to flag outlier pixels. First create the object
    # TODO: confirm/explore best input paramters based on realistic data

    # set one kappa to a very large value to separate hot/cold maxing

    max_iter = 6
    filter_x = 5
    filter_y = 5
    bpFind = hdrl.func.BPM2D.Filter(1000, kappa_high, max_iter, cpl.core.Filter.MEDIAN, cpl.core.Border.NOP,
                                    filter_x, filter_y)
    mask_hot = bpFind.compute(image)
    bpFind = hdrl.func.BPM2D.Filter(kappa_low, 1000, max_iter, cpl.core.Filter.MEDIAN, cpl.core.Border.NOP,
                                    filter_x, filter_y)
    mask_cold = bpFind.compute(image)

    return mask_hot, mask_cold
