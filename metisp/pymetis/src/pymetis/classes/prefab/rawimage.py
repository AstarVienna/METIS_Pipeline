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

import numpy as np

from abc import ABC
from typing import Literal, Optional

import cpl
from cpl.core import Msg, Image, ImageList

from pymetis.classes.recipes import MetisRecipeImpl
from pymetis.classes.inputs import PipelineInputSet, RawInput


CombineMethodType = Literal['add', 'average', 'median', 'sigclip']


class RawImageProcessor(MetisRecipeImpl, ABC):
    """
    RawImageProcessor is a recipe implementation that takes a bunch of raw frames,
    categorizes them according to their properties and outputs and performs a sanity check or two.
    """

    class InputSet(PipelineInputSet):
        RawInput: type[RawInput] = RawInput

    @classmethod
    def combine_images(cls,
                       images: cpl.core.ImageList,
                       method: CombineMethodType) -> cpl.core.Image:
        """
        Basic helper method to combine images using one of `add`, `average`, `median` or `sigclip`.
        Probably not a panacea, but it recurs often enough to warrant being here.

        Raises
        ------
        ValueError
            If an unknown combine method is used.

        """
        Msg.info(cls.__qualname__,
                 f"Combining {len(images)} images using method {method!r}")
        combined_image: Optional[cpl.core.Image] = None

        match method:
            case "add":
                for idx, image in enumerate(images):
                    if idx == 0:
                        combined_image = image
                    else:
                        combined_image.add(image)
            case "average":
                combined_image = images.collapse_create()
            case "median":
                combined_image = images.collapse_median_create()
            case "sigclip":
                combined_image = images.collapse_sigclip_create()
            case _:
                Msg.error(cls.__qualname__,
                          f"Got unknown stacking method {method!r}. Stopping right here!")
                raise ValueError(f"Unknown stacking method {method!r}")

        return combined_image

    @classmethod
    def combine_images_with_error(cls,
                                  images: ImageList,
                                  method: CombineMethodType,
                                  read_noise: float) -> tuple[Image, Image]:
        """
        Collapse and imagelist of raw frames and propagate the errors

        Parameters
        ----------
        images : ImageList
            List of raw images to combine
        method : CombineMethodType = Literal['add', 'average', 'median', 'sigclip']
            Method to combine images using one of `add`, `average`, `median` or `sigclip`.
        read_noise : float
            Read noise # ToDo what does this mean precisely?

        Returns
        -------
        combined_image : Image
            Combined image
        """
        combined_image = cls.combine_images(images, method)
        # for each image, calculate the noise (read noise + shot noise, added in quadrature)
        error = Image.zeros_like(images[0])

        for im in images:
            # shot noise as sqrt of signal in, after applying gain
            poisson_noise = cpl.core.Image.zeros_like(im)
            poisson_noise.copy_into(im, 0, 0)

            # add read noise plus shot noise
            totalNoise = cpl.core.Image.zeros_like(poisson_noise)
            totalNoise.add(poisson_noise)
            totalNoise.add_scalar(read_noise ** 2)

            # this is square of the noise; add to a running total
            error.add(totalNoise)

        # and take the sqrt
        error.power(0.5)
        error.divide_scalar(np.sqrt(len(images)))

        return combined_image, error


    def correct_gain(self, raw_images: ImageList, gain: Image) -> ImageList:
        """
        Correct the raw image list for gain.

        [FIXME] currently a mockup, does not actually correct gain.

        Parameters
        ----------
        raw_images : ImageList
            List of raw images to correct

        Returns
        -------
        corrected_images : ImageList
            List of gain-corrected images
        """
        Msg.info(self.__class__.__qualname__,
                 f"Pretending to correct raw images for gain")

        raw_images.divide_image(gain)

        return raw_images


    def correct_nonlinearity(self, raw_images: ImageList, linearity_map: Image) -> ImageList:
        """
        Correct the raw image list for non-linearity.

        # FixMe Currently only a mockup, does not actually do anything.

        Parameters
        ----------
        raw_images : ImageList
            List of raw images to correct for nonlinearity.

        Returns
        -------
        ImageList
            List of raws, now corrected for non-linearity.
        """
        Msg.info(self.__class__.__qualname__, f"Pretending to correct for non-linearity")
        return raw_images

    def calculate_outliers(self,
                           image: Image,
                           *,
                           kappa_low: int,
                           kappa_high: int) -> tuple[cpl.core.Mask, cpl.core.Mask]:
        """
        Calculate masks for outlier pixels, with kappa-sigma clipping:
        mask those values are outside [median - kappa_low * sigma, median + kappa_high * sigma].
        """
        Msg.info(self.__class__.__qualname__,
                 f"Identifying outlier pixels ({kappa_low=}, {kappa_high=})")

        # get masks from thresholds for bad, hot and cold pixels
        # count the number of bad pixels in each, for later, then
        # change to Image type from mask for later calculations

        image_median = image.get_median()
        image_rms = image.get_stdev()

        # ToDo: why is this not the other way around? Set everything above threshold to 0...
        mask_hot = cpl.core.Mask.threshold_image(image, 0, image_median + kappa_high * image_rms, 1)
        # ToDo ...and then here everything below threshold to 0 too. Would be more consistent maybe?
        mask_cold = cpl.core.Mask.threshold_image(image, 0, image_median - kappa_low * image_rms, 0)

        return mask_hot, mask_cold

    def metis_bpm_3d_compute(self,
                             imagelist: ImageList,
                             *,
                             kappa_low: float,
                             kappa_high: float) -> cpl.core.Mask:
        """
        Calculate mask for outlier pixels based on high/low thresholds based on the frame to frame variation of a pixel.

        ToDo detailed description

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

        image_sum = Image.zeros_like(imagelist[0])
        image_sum_squared = Image.zeros_like(imagelist[0])

        # image_sum = self.combine_images(imagelist, method="add")

        for im in imagelist:
            imTem = cpl.core.Image.zeros_like(im)
            # copy_into means "copy other into this image", not the other way around!
            imTem.copy_into(im, 0, 0)
            image_sum.add(im)
            im.power(2)
            image_sum_squared.add(im)

        image_sum.divide_scalar(len(imagelist))
        image_sum.power(2)
        image_sum_squared.divide_scalar(len(imagelist))

        image_sum.add(image_sum_squared)
        image_sum.power(0.5)

        image_median = image_sum.get_median()
        image_rms = image_sum.get_stdev()

        mask = cpl.core.Mask.threshold_image(image_sum,
                                             image_median - kappa_low * image_rms,
                                             image_median + kappa_high * image_rms,
                                             1)
        return mask

