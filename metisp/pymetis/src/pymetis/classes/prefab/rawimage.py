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


COMBINE_METHOD = Literal['add'] | Literal['average'] | Literal['median'] | Literal['sigclip']


class RawImageProcessor(MetisRecipeImpl, ABC):
    """
    RawImageProcessor is a recipe implementation that takes a bunch of raw frames,
    categorizes them according to their properties and outputs and performs a sanity check or two.
    """

    class InputSet(PipelineInputSet):
        RawInput: type[RawInput] = RawInput
        detector: str = None

    @classmethod
    def combine_images(cls,
                       images: cpl.core.ImageList,
                       method: COMBINE_METHOD) -> cpl.core.Image:
        """
        Basic helper method to combine images using one of `add`, `average`, `median` or `sigclip`.
        Probably not a universal panacea, but it recurs often enough to warrant being here.
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
                combined_image = images.collapse_sigclip()
            case _:
                Msg.error(cls.__qualname__,
                          f"Got unknown stacking method {method!r}. Stopping right here!")
                raise ValueError(f"Unknown stacking method {method!r}")

        return combined_image

    @classmethod
    def combine_images_with_error(cls,
                                  images: ImageList,
                                  method: COMBINE_METHOD,
                                  read_noise: float) -> tuple[Image, Image]:
        """
        Collapse and imagelist of raw frames and propagate the errors
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

