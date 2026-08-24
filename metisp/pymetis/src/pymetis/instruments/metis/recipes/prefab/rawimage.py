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

from abc import ABC
from typing import Literal

import cpl, hdrl
from cpl.core import Msg, Image as CplImage, ImageList as CplImageList
from cpl.hdrl.core import ImageList as HdrlImageList

from pymetis.drl.combine import combine_images
from pymetis.drl.image import zeros_like
from pymetis.engine.recipes import RecipeImpl
from pymetis.engine.inputs import PipelineInputSet

from ...inputs import RawInput, BadPixMapInput, OptionalInputMixin

CombineMethodType = Literal['add', 'average', 'median', 'sigclip']


class RawImageProcessor(RecipeImpl, ABC):
    """
    RawImageProcessor is a recipe implementation that takes a bunch of raw frames,
    categorizes them according to their properties and outputs and performs a sanity check or two.
    """

    class InputSet(PipelineInputSet):
        class BadPixMapInput(OptionalInputMixin, BadPixMapInput):
            pass

        raw: RawInput
        bad_pix_map: BadPixMapInput

    @classmethod
    def apply_mask(cls,
                   cplImage: cpl.core.Image | hdrl.core.Image,
                   cplMask: cpl.core.Image,
                   bits: list) -> cpl.core.Image | hdrl.core.Image:
        """
            Given a mask in 32 bit cplImage form, a list of bit values,
            and an hdrl or cpl image, 
            extract the bits from the mask, create a cpl mask based on it,
            and apply to the image. 
        """

        maskFrame = zeros_like(cplMask, cpl.core.Type.INT)

        # go through, get the required bits and add to the mask frame
        for bit in bits:
            temp = cpl.core.Image.zeros_like(cplMask)
            temp.copy_into(cplMask, 0, 0)
            temp.and_scalar(bit)
            maskFrame.add(temp)

        # create a mask object
        mask = cpl.core.Mask(cplImage.width, cplImage.height)
        
        # a bit kludgy, but creating a numpy boolean array with the 
        # required mask values, then directly assigning it in the way
        # pycpl accepts indices. TODO get pycpl native way of doing this. 
        
        isTrue = maskFrame.as_array().astype(bool)
        mask[0:temp.width][0:temp.height] = isTrue
        
        cplImage.reject_from_mask(mask)

        return cplImage

    @classmethod
    def update_mask(cls, mask: cpl.core.Mask,
                    bitVal: int,
                    cplMask: cpl.core.Image) -> cpl.core.Image:

        """ 
        given a cpl image mask, and a bit value, add them to a CPL bit mask. 

        If you're starting with an image that has a mask, pass

            image.bpm: for cpl image
            image.data.bpm for hdrl image

        """

        mask = mask.as_array()
        
        #if(isinstance(image,hdrl.core.Image)):
        #   mask = image.image.bpm.as_array()
        #else:
        #   mask = image.bpm.as_array()

        
        # turn it into a CPL image
        update = cpl.core.Image(mask,dtype=cpl.core.Type.INT)

        # multiply it by the bit value

        update.multiply_scalar(bitVal)
        cplMask.add(update)
        
        return cplMask

    def combine_images(self, images: CplImageList, method: CombineMethodType) -> CplImageList:
        """ Temporary wrapper, use the function directly in the future. """
        return combine_images(images, method)

    def correct_gain(self, raw_images: CplImageList, gain: CplImage) -> CplImageList:
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
                 "Pretending to correct raw images for gain")

        raw_images.divide_image(gain)

        return raw_images


    def correct_nonlinearity(
            self,
            raw_images: CplImageList,
            linearity_map: CplImageList,
        ) -> CplImageList:
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
        Msg.info(self.__class__.__qualname__, "Pretending to correct for non-linearity")
        return raw_images

    def calculate_outliers_sequence(self,
                             imagelist: CplImageList | HdrlImageList,
                             *,
                             kappa_low: float,
                             kappa_high: float) -> cpl.core.Mask:
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

        image_sum = hdrl.core.Image.zeros(imagelist[0].width,imagelist[0].height)
        image_sum_squared = hdrl.core.Image.zeros(imagelist[0].width,imagelist[0].height)

        for im in imagelist:
            image_sum.add_image(im)
            im.pow_scalar((2,0))
            image_sum_squared.add_image(im)

        image_sum.div_scalar((len(imagelist),0))
        image_sum.pow_scalar((2,0))
        image_sum_squared.div_scalar((len(imagelist),0))

        image_sum.add_image(image_sum_squared)
        image_sum.pow_scalar((0.5,0))

        image_median = image_sum.get_median()
        image_rms = image_sum.get_stdev()
        
        mask = cpl.core.Mask.threshold_image(image_sum.image,
                                                    image_median[0] - kappa_low * 1 * image_rms,
                                                    image_median[0] + kappa_high * 1 * image_rms,
                                                    0)
        return mask

