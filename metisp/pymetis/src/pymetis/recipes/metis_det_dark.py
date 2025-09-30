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
from typing import Self

import cpl
from cpl.core import Msg
from pyesorex.parameter import ParameterList, ParameterEnum

from pymetis.classes.dataitems import DataItem
from pymetis.dataitems.masterdark.masterdark import MasterDark
from pymetis.dataitems.masterdark.raw import DarkRaw
from pymetis.classes.inputs import (RawInput, BadPixMapInput, PersistenceMapInput,
                                    LinearityInput, GainMapInput, OptionalInputMixin)
from pymetis.classes.prefab import RawImageProcessor
from pymetis.classes.recipes import MetisRecipe

import numpy as np

class MetisDetDarkImpl(RawImageProcessor, ABC):
    """
    Implementation class for `metis_det_dark`.
    """

    # We start by deriving the implementation class from `MetisRecipeImpl`, or in this case, one of its subclasses,
    # namely `RawImageProcessor, as this recipe processes raw images and we would like to reuse the functionality.

    # First of all, we need to define the input set. Since we are deriving from `RawImageProcessor`,
    # we mayreuse the `InputSet` class from it too. This automatically adds a `RawInput` for us.
    class InputSet(RawImageProcessor.InputSet):
        """
        InputSet class for `metis_det_dark`.
        """

        # However, we still need to define the tags on the class level.
        # Therefore, we override the `_tags` attribute and also the description,
        # since this is specific to this raw input, not all raw inputs.
        class RawInput(RawInput):
            Item = DarkRaw

        # Next, we define all other input classes using predefined ones.
        # Here we mark them as optional, but if we did not need that, we could have also said
        # ```PersistenceMapInput = PersistenceMapInput```
        # to tell the class that its persistence map input is just the global `PersistenceMapInput` class.
        class PersistenceMapInput(OptionalInputMixin, PersistenceMapInput):
            pass

        class BadPixMapInput(OptionalInputMixin, BadPixMapInput):
            pass

        # FixMe: these two should **not** be optional, but the current EDPS workflow does not supply them
        class LinearityInput(OptionalInputMixin, LinearityInput):
            pass

        class GainMapInput(OptionalInputMixin, GainMapInput):
            pass

    ProductMasterDark = MasterDark

    # At this point, we should have all inputs and outputs defined -- the "what" part of the recipe implementation.
    # Now we define the "how" part, or the actions to be performed on the data.
    # See the documentation of the parent's `process` function for more details.
    # Feel free to define other functions to break up the algorithm into more manageable chunks
    # and call them from within `process` as needed.


    ########################################################################
    # TODO?? and outstanding issues
    #
    # DRLD specifies hdrml_bpm_3d_compute, which implies finding outlying pixels on a stack
    # of images, which requires a sufficient number of input files. Is this checked?
    # at the moment, using sigma clipping of the final image to identify outlier pixels.
    #
    # consolidate bit values of the mask with other recipes
    #
    # Once multi-extensions supported, read bitmask from DETLIN to intialize
    #
    # Noise and bad pixel masks are not yet written to file.
    #
    # Once multi-extension input/output is implemented, we need to extend the code to the
    # IFU (w/ four detectors) and properly read in the gain/badpix.
    #
    # Check sigmas for thresholds / combining
    #
    # what exactly do we mean by "bad pixel" compared to hot or cold; check interpretation.
    #
    # Also, persistence and non-linearity to be implemented.
    #########################################################################

    def correct_persistence(self) -> Self:
        Msg.info(self.__class__.__qualname__, f"Pretending to do persistence correction")
        return self

    def correct_nonlinearity(self) -> Self:
        Msg.info(self.__class__.__qualname__, f"Pretending to correct for non-linearity")
        return self

    def process(self) -> set[DataItem]:
        method = self.parameters["metis_det_dark.stacking.method"].value

        # load calibration files

        # TODO: preprocessing steps like persistence correction / nonlinearity (or not)
        self.inputset.raw.load_data()
        self.inputset.raw.use()

        Msg.info(self.__class__.__qualname__, f"Loading raw dark data")
        raw_images = cpl.core.ImageList([r.hdus[0] for r in self.inputset.raw.items])
        combined_image = self.combine_images(raw_images, method)
        header = self.inputset.raw.items[0].header

        # load raw data
        Msg.info(self.__class__.__qualname__, f"{len(raw_images)} raw dark frames loaded")


        Msg.info(self.__class__.__qualname__, f"Pretending to load DETLIN")

        Msg.info(self.__class__.__qualname__, f"Faking a gain map and badpix map")

        # fake the bp mask by initializing to zero
        temp = cpl.core.Image.zeros_like(raw_images[0])
        bpMask = temp.cast(cpl.core.Type.INT)

        # fake the gain at the moment by setting to 1

        gain = cpl.core.Image.zeros_like(raw_images[0])
        gain.add_scalar(1)

        # correcting for gain
        Msg.info(self.__class__.__qualname__, f"Correcting raw images for gain")

        raw_images.divide_image(gain)


        # now calculate the readnoise

        if len(raw_images) > 1:
            Msg.info(self.__class__.__qualname__, f"Calculating read noise from {len(raw_images)} raw dark frames")
            diff = cpl.core.Image(raw_images[0])
            diff.subtract(raw_images[1])
            readNoise = cpl.drs.detector.get_noise_window(diff, None)
            combined_image, noise = self.combine_images_with_error(raw_images, method, readNoise[0])
        else:
            Msg.warning(self.__class__.__qualname__, f"Cannot calculate read noise as there is only one raw image")

        self.correct_persistence()
        self.correct_nonlinearity()

        Msg.info(self.__class__.__qualname__, f"Combining images using method {method!r}")

        Msg.info(self.__class__.__qualname__, "Calculate outlying pixels")

        darkRms = combined_image.get_stdev()
        darkMedian = combined_image.get_median()

        # get masks from thresholds for bad, hot and cold pixels
        # count the number of bad pixels in each, for later, then
        # change to Image type from mask for later calculations

        imMed = combined_image.get_median()
        imRMS = combined_image.get_stdev()

        maskHot = cpl.core.Mask.threshold_image(combined_image, 0, imMed + kappaHigh*imRMS, 1)
        qcnhot = maskHot.count()
        maskHot = cpl.core.Image(maskHot,dtype = cpl.core.Type.INT)

        maskCold = cpl.core.Mask.threshold_image(combined_image, 0, imMed - kappaLow*imRMS, 0)
        qcncold = maskCold.count()
        maskCold = cpl.core.Image(maskCold, dtype=cpl.core.Type.INT)

        maskBad, qcnbad =  self.metis_bpm_3d_compute(raw_images, kappaLow, kappaHigh)

        # multiple masks to the correct bitmask
        maskBad.multiply_scalar(badBit)
        maskCold.multiply_scalar(coldBit)
        maskHot.multiply_scalar(hotBit)

        # and update main mask
        bpMask.add(maskBad)
        bpMask.add(maskHot)
        bpMask.add(maskCold)

        Msg.info(self.__class__.__qualname__, "Updating Mask; {mask.count()} pixels set as bad")

        ## how to copy mask into image?

        Msg.info(self.__class__.__qualname__, "Actually Calculating QC Parameters")

        # calculate the stats in each individual image
        medians = []
        means = []
        stdevs = []
        mins = []
        maxs = []
        for im in raw_images:
            medians.append(im.get_median())
            means.append(im.get_mean())
            stdevs.append(im.get_stdev())
            mins.append(im.get_min())
            maxs.append(im.get_max())

        qcmed = combined_image.get_median()
        qcmean = combined_image.get_mean()
        qcrms = combined_image.get_stdev()

        qcncoadd = len(raw_images)

        qcmedmed = np.median(np.array(medians))
        qcmedrms = np.median(np.array(stdevs))
        qcmedmin = np.median(np.array(mins))
        qcmedmax = np.median(np.array(maxs))
        qcmedmean = np.median(np.array(means))

        header = cpl.core.PropertyList.load(self.inputset.raw.frameset[0].file, 0)
        Msg.info(self.__class__.__qualname__, "Appending QC Parameters to header")

        header.append(cpl.core.Property("QC DARK MEAN", cpl.core.Type.DOUBLE,
                                        qcmean, "[ADU] mean value of master dark"))
        header.append(cpl.core.Property("QC DARK MEDIAN", cpl.core.Type.DOUBLE,
                                        qcmed, "[ADU] median value of master dark"))
        header.append(cpl.core.Property("QC DARK RMS", cpl.core.Type.DOUBLE,
                                        qcrms, "[ADU] rms value of master dark"))
        header.append(cpl.core.Property("QC DARK NBADPIX", cpl.core.Type.DOUBLE,
                                        qcnbad, "[ADU] number of bad pixels"))
        header.append(cpl.core.Property("QC DARK NCOLDPIX", cpl.core.Type.DOUBLE,
                                        qcncold, "[ADU] number of cold pixels"))
        header.append(cpl.core.Property("QC DARK NHOTPIX", cpl.core.Type.DOUBLE,
                                        qcnhot, "[ADU] number of hot pixels"))
        header.append(cpl.core.Property("QC DARK MEDIAN MEAN", cpl.core.Type.DOUBLE,
                                        qcmedmean, "[ADU] median value of mean values of individual input images"))
        header.append(cpl.core.Property("QC DARK MEDIAN MEDIAN", cpl.core.Type.DOUBLE,
                                        qcmedmed, "[ADU] median value of median values of individual input images"))
        header.append(cpl.core.Property("QC DARK MEDIAN RMS", cpl.core.Type.DOUBLE,
                                        qcmedrms, "[ADU] median value of RMS values of individual input images"))
        header.append(cpl.core.Property("QC DARK MEDIAN MIN", cpl.core.Type.DOUBLE,
                                        qcmedmin, "[ADU] median value of min values of individual input images"))
        header.append(cpl.core.Property("QC DARK MEDIAN MAX", cpl.core.Type.DOUBLE,
                                         qcmedmax, "[ADU] median value of max values of individual input images"))

        product = self.ProductMasterDark(header, combined_image)

        return {product}


    def metis_bpm_3d_compute(self,imageList,kappaLow, kappaHigh):

        """Calculate outlier pixels based on high/low thresholds based on the frame to frame variation of a pixel"""

        imsum = cpl.core.Image.zeros_like(imageList[0])
        im2sum = cpl.core.Image.zeros_like(imageList[0])

        for im in imageList:
            imTem = cpl.core.Image.zeros_like(im)
            imTem.copy_into(im, 0, 0)
            imsum.add(im)
            im.power(2)
            im2sum.add(im)

        imsum.divide_scalar(len(imageList))
        imsum.power(2)
        im2sum.divide_scalar(len(imageList))

        imsum.add(im2sum)
        imsum.power(0.5)

        imMed = imsum.get_median()
        imRMS = imsum.get_stdev()

        mask = cpl.core.Mask.threshold_image(imsum, imMed - kappaLow*imRMS, imMed + kappaHigh*imRMS, 1)
        qcnbad = mask.count()
        mask = cpl.core.Image(mask,dtype = cpl.core.Type.INT)

        return mask, qcnbad




# This is the actual recipe class that is visible by `pyesorex`.
class MetisDetDark(MetisRecipe):
    # Fill in recipe information for `pyesorex`. These are required and checked by `pyesorex`.
    _name = "metis_det_dark"
    _version = "0.1"
    _author = "Hugo Buddelmeijer, A*"
    _email = "hugo@buddelmeijer.nl"
    _synopsis = "Create master dark"
    _description = (
        "Prototype to create a METIS masterdark."
    )

    # And also fill in information from DRLD. These are specific to METIS and are used to build the description
    # for the man page. Later, we would like to be able to compare them directly to DRLD and test for that.
    _matched_keywords = set()
    _algorithm = """
        - Group files by detector and `DIT`, based on header keywords
        - Call function `metis_determine_dark` for each set of files
        - Call `metis_update_dark_mask` to flag deviant pixels
    """

    # Define the parameters as required by the recipe. Again, this is needed by `pyesorex`.
    parameters = ParameterList([
        ParameterEnum(
            name=f"{_name}.stacking.method",
            context=_name,
            description="Name of the method used to combine the input images",
            default="average",
            alternatives=("add", "average", "median", "sigclip"),
        ),
    ])

    # Point the `implementation_class` to the *top* class of your recipe hierarchy.
    # All promotions should happen at instantiation time.
    Impl = MetisDetDarkImpl
