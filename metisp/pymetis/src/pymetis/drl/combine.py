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

from cpl.core import Image as CplImage, ImageList as CplImageList, Msg
from hdrl.core import Image as HdrlImage, ImageList as HdrlImageList
#import hdrl

from typing import Literal, Optional

CombineMethodType = Literal['add', 'average', 'median', 'sigclip']


def combine_images_hdrl(
        images: HdrlImageList,
        method: CombineMethodType
) -> HdrlImage:
    Msg.info("combine_images_hdrl",
             f"Combining HDRL {len(images)} images using method {method!r}")
    combined_image: Optional[HdrlImage] = None

    match method:
        case "add":
            for idx, image in enumerate(images):
                if idx == 0:
                    combined_image = image
                else:
                    # HDRL calls this function `add_image`, not just `add`
                    combined_image.add_image(image)
        case "average":
            combined_image, coverage_map = images.collapse_mean()
        case "median":
            combined_image, coverage_map = images.collapse_median()
        case "sigclip":
            combined_image, coverage_map = images.collapse_sigclip()
        case _:
            raise ValueError(f"Unknown stacking method {method!r}")

    # HDRL also produces coverage maps, we ignore those for now
    return combined_image


def combine_images_cpl(
        images: CplImageList,
        method: CombineMethodType
) -> CplImage:
    """
    Basic helper method to combine images using one of `add`, `average`, `median` or `sigclip`.
    Probably not a panacea, but it recurs often enough to warrant being here.

    Raises
    ------
    ValueError
        If an unknown combine method is used.

    """
    Msg.info("combine_images_cpl",
             f"Combining CPL {len(images)} images using method {method!r}")
    combined_image: Optional[CplImage] = None

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
            raise ValueError(f"Unknown stacking method {method!r}")

    return combined_image

def combine_images(
        images: CplImageList | HdrlImageList,
        method: CombineMethodType
) -> CplImage | HdrlImage:
    """
    Basic helper method to combine images using one of `add`, `average`, `median` or `sigclip`.
    Probably not a panacea, but it recurs often enough to warrant being here.

    Calls one of two sub-methods based on whether it's an HDRL or CPL image, as the
    processes are teh same, but the names of the associated methods as different.

    Raises
    ------
    ValueError
        If an unknown combine method is used.
        If the wrong input type is passed
    """

    Msg.info("combine_images",
             f"Combining {len(images)} images using method {method!r}")

    if isinstance(images, HdrlImageList):
        combined_image = combine_images_hdrl(images, method)
    elif isinstance(images, CplImageList):
        combined_image = combine_images_cpl(images, method)
    else:
        raise ValueError(f"Unknown input type {type(images)}")

    return combined_image

