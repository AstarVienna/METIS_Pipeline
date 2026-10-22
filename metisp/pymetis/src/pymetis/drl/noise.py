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

import numpy as np

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
        read_noise: float,
    ) -> HdrlImage:
    """
    Turn a raw CPL image into an HDRL image with basic noise estimates.
    """

    Msg.info("estimate_noise",
             f"Estimating noise for raw image, readnoise = {read_noise}")

    noise = zeros_like(image, bpm=image.bpm)
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


def calculate_outliers_sequence(
        images: HdrlImageList,
        *,
        kappa_low: float,
        kappa_high: float,
    ) -> CplMask:
    """
    Flag pixels whose frame-to-frame scatter is anomalous.

    The standard deviation of every pixel across the sequence is compared with the
    median and standard deviation of that scatter image: pixels more than `kappa_high`
    sigma noisier, or `kappa_low` sigma quieter, than the typical pixel are flagged.
    The input images are not modified.
    """
    Msg.info("calculate_outliers_sequence",
             f"Identifying noisy pixels across {len(images)} frames ({kappa_low=}, {kappa_high=})")

    stack = np.stack([np.asarray(image.image.as_array(), dtype=float) for image in images])
    scatter = stack.std(axis=0)
    median, rms = np.median(scatter), scatter.std()

    return CplMask((scatter < median - kappa_low * rms) | (scatter > median + kappa_high * rms))


def calculate_outliers_sequence(
        self,
        imagelist: CplImageList | HdrlImageList,
        *,
        kappa_low: float,
        kappa_high: float
    ) -> cpl.core.Mask:
    """
    Calculate mask for outlier pixels based on high/low thresholds based on the frame to frame variation of a pixel.

    Need to think about exactly how we do this, as we can be dealing with relatively small numbers of input frames.
    An RMS that works for hot/cold pixels may be too conservative for this.

    Parameters
    ----------
    imagelist : ImageList
        List of raw images to combine

    kappa_low : float
        Lower bound of kappa for outlier pixels

    kappa_high : float
        Upper bound of kappa for outlier pixels

    Returns
    -------
    mask : cpl.core.Mask
        Mask for outlier pixels.
    """
    Msg.info(self.__class__.__qualname__,
             f"Calculating bad pixel mask ({kappa_low=}, {kappa_high=})")

    image_sum = hdrl.core.Image.zeros(imagelist[0].width, imagelist[0].height)
    image_sum_squared = hdrl.core.Image.zeros(imagelist[0].width, imagelist[0].height)

    for im in imagelist:
        image_sum.add_image(im)
        im.pow_scalar_create((2, 0))
        image_sum_squared.add_image(im)

    image_sum.div_scalar((len(imagelist), 0))
    image_sum.pow_scalar((2, 0))
    image_sum_squared.div_scalar((len(imagelist), 0))

    image_sum.sub_image(image_sum_squared)
    image_sum.pow_scalar((0.5, 0))

    image_median = image_sum.get_median()
    image_rms = image_sum.get_stdev()

    mask = cpl.core.Mask.threshold_image(
        image_sum.image,
        image_median[0] - kappa_low * 1 * image_rms,
        image_median[0] + kappa_high * 1 * image_rms,
        False,
    )
    return mask
