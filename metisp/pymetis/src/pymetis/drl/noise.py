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

    The scatter is computed in two passes (mean first, then the mean squared
    deviation) rather than as `mean(x**2) - mean(x)**2`, which cancels
    catastrophically for the large pixel values of a raw frame.

    Parameters
    ----------
    images : hdrl.core.ImageList
        Frames of the sequence, with their noise estimates.

    kappa_low, kappa_high : float
        Acceptance band around the median scatter, in units of its standard deviation.

    Returns
    -------
    cpl.core.Mask
        True where the pixel is an outlier.
    """
    Msg.info("calculate_outliers_sequence",
             f"Identifying noisy pixels across {len(images)} frames ({kappa_low=}, {kappa_high=})")

    n = len(images)
    width, height = images[0].width, images[0].height

    mean = HdrlImage.zeros(width, height)
    for image in images:
        mean.add_image(image)
    mean.div_scalar((n, 0))

    scatter = HdrlImage.zeros(width, height)
    for image in images:
        deviation = image.sub_image_create(mean)
        deviation.pow_scalar((2, 0))
        scatter.add_image(deviation)
    scatter.div_scalar((n, 0))
    scatter.pow_scalar((0.5, 0))

    median = scatter.get_median().data
    rms = scatter.get_stdev()

    too_quiet = CplMask.threshold_image(scatter.image, -math.inf, median - kappa_low * rms, True)
    too_noisy = CplMask.threshold_image(scatter.image, median + kappa_high * rms, math.inf, True)

    return too_quiet | too_noisy


def calculate_outliers_sequence_np(
        images: HdrlImageList,
        *,
        kappa_low: float,
        kappa_high: float,
    ) -> CplMask:
    """
    numpy reference implementation of `calculate_outliers_sequence`.

    Same contract: flag pixels whose frame-to-frame standard deviation lies outside
    `median ± kappa * rms` of the scatter image. The input images are not modified.
    """
    Msg.info("calculate_outliers_sequence_np",
             f"Identifying noisy pixels across {len(images)} frames ({kappa_low=}, {kappa_high=})")

    stack = np.stack([np.asarray(image.image.as_array(), dtype=float) for image in images])
    scatter = stack.std(axis=0)
    median, rms = np.median(scatter), scatter.std()

    return CplMask((scatter < median - kappa_low * rms) | (scatter > median + kappa_high * rms))
