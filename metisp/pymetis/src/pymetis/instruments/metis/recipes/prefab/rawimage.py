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

import cpl, hdrl
from cpl.core import Msg, Image, ImageList

from pymetis.engine.recipes import RecipeImpl
from pymetis.engine.inputs import PipelineInputSet

from pymetis.instruments.metis.inputs import RawInput, BadPixMapInput, OptionalInputMixin

CombineMethodType = Literal['add', 'average', 'median', 'sigclip']


class RawImageProcessor(RecipeImpl, ABC):
    """
    RawImageProcessor is a recipe implementation that takes a bunch of raw frames,
    categorizes them according to their properties and outputs and performs a sanity check or two.
    """

    class InputSet(PipelineInputSet):
        RawInput: type[RawInput] = RawInput

        class BadPixMapInput(OptionalInputMixin, BadPixMapInput):
            pass

    @classmethod
    def estimate_noise_list(cls,
                       images: cpl.core.ImageList,
                       read_noise: float)-> hdrl.core.ImageList:
        """
        Routine to turn a cpl Imagelist of raw images into a hdrl ImageList with 
        basic noise estimates.
        """
        images_hdrl = hdrl.core.ImageList()

        for im in images:
            image_hdrl = cls.estimate_noise(im, read_noise)

            images_hdrl.append(image_hdrl)

        return images_hdrl

        
    @classmethod
    def estimate_noise(cls,
                       image: cpl.core.Image,
                       read_noise: float) -> hdrl.core.Image:

        """
        Routine to turn a raw cpl image into an hdrl image with 
        basic noise estimates.
        """

        Msg.info(cls.__qualname__,
                 f"Estimating noise for raw image, readnoise = {read_noise}")

        noise = cpl.core.Image.zeros_like(image)
        noise.copy_into(image, 0, 0)

        # add read noise plus shot noise
        noise.add_scalar(read_noise ** 2)
        noise.power(0.5)

        image_hdrl = hdrl.core.Image(image,noise)
        
        return image_hdrl

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

        maskFrame = cpl.core.Image.zeros_like(cplMask)

        # go through, get the required bits and add to the mask frame
        for bit in bits:
            temp = cpl.core.Image.zeros_like(cplMask)
            temp.copy_into(cplMask,0,0)
            temp.and_scalar(bit)
            maskFrame.add(temp)

        # create a mask object
        mask = cpl.core.Mask(cplImage.width,cplImage.height)
        
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
    
    @classmethod
    def _combine_images_hdrl(cls,
                       images: hdrl.core.ImageList,
                       method: CombineMethodType) -> cpl.core.Image:

        combined_image: Optional[cpl.core.Image] = None

        match method:
            case "add":
                for idx, image in enumerate(images):
                    if idx == 0:
                        combined_image = image
                    else:
                        combined_image.add(image)
            case "average":
                combined_image, coverage_map = images.collapse_mean()
            case "median":
                combined_image, coverage_map = images.collapse_median()
            case "sigclip":
                combined_image, coverage_map = images.collapse_sigclip()
            case _:
                Msg.error(cls.__qualname__,
                          f"Got unknown stacking method {method!r}. Stopping right here!")
                raise ValueError(f"Unknown stacking method {method!r}")
            
        return combined_image

             
    @classmethod
    def _combine_images_cpl(cls,
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
    def combine_images(cls,
                            images: hdrl.core.ImageList | cpl.core.ImageList,
                            method: CombineMethodType) -> hdrl.core.Image | cpl.core.Image:

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
        
        Msg.info(cls.__qualname__,
                 f"Combining {len(images)} images using method {method!r}")

        combined_image: Optional[cpl.core.Image] = None

        # separate submethods, because the syntax/method is different for
        # hdrl and cpl images
        
        if(isinstance(images,hdrl.core.ImageList)):
           combined_image = cls._combine_images_hdrl(images,method)
        elif(isinstance(images,cpl.core.ImageList)):
           combined_image = cls._combine_images_hdrl(images,method)
        else:
             Msg.error(cls.__qualname__,
                       f"Unknown input type {type(images)}. Stopping right here!")
             raise ValueError(f"Unknown input type {type(images)}")

        return combined_image



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
                           image: hdrl.core.Image,
                           *,
                           kappa_low: int,
                           kappa_high: int) -> tuple[cpl.core.Mask, cpl.core.Mask]:
        """
        Calculate masks for outlier pixels, with kappa-sigma clipping, using HDRL BPM functions.
        """
        Msg.info(self.__class__.__qualname__,
                 f"Identifying outlier pixels ({kappa_low=}, {kappa_high=})")

        # get masks from thresholds for bad, hot and cold pixels
        # count the number of bad pixels in each, for later, then
        # change to Image type from mask for later calculations

        image_median = image.get_median()
        image_rms = image.get_stdev()

        # user HDRL function to flag outlier pixels. First create the object
        # TODO: confirm/explore best input paramters based on realistic data

        #set one kappa to a very large value to separate hot/cold maxing
        
        maxIter = 6
        filter_x = 5
        filter_y = 5
        bpFind = hdrl.func.BPM2D.Filter(1000, kappa_high, maxIter, cpl.core.Filter.MEDIAN, cpl.core.Border.NOP, filter_x, filter_y)
        mask_hot = bpFind.compute(image)
        bpFind = hdrl.func.BPM2D.Filter(kappa_low, 1000, maxIter, cpl.core.Filter.MEDIAN, cpl.core.Border.NOP, filter_x, filter_y)
        mask_cold = bpFind.compute(image)

        return mask_hot, mask_cold

    def calculate_outliers_sequence(self,
                             imagelist: cpl.core.ImageList | hdrl.core.ImageList,
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
istory            Lower bound of kappa for outlier pixels

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

