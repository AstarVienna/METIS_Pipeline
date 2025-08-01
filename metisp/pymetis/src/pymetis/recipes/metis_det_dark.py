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

import cpl
from cpl.core import Msg
from pyesorex.parameter import ParameterList, ParameterEnum

from pymetis.classes.dataitems import DataItem
from pymetis.classes.dataitems.masterdark.masterdark import MasterDark
from pymetis.classes.dataitems.masterdark.raw import DarkRaw
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


    def process(self) -> set[DataItem]:

        badBit = 2
        hotBit = 4
        coldBit = 8
        
        
        method = self.parameters["metis_det_dark.stacking.method"].value

        Msg.info(self.__class__.__qualname__, f"Pretending to Load DETLIN")
        #linearity = cpl.core.Image.load(self.inputset.gain_map.frame.file, extension=0)
        
        Msg.info(self.__class__.__qualname__, f"Pretending to load GAIN_MAP")
        #gain = cpl.core.Image.load(self.inputset.gain_map.frame.file, extension=0)
        
        Msg.info(self.__class__.__qualname__, f"Loading raw dark data")
        raw_images = self.inputset.load_raw_images()
        
        Msg.info(self.__class__.__qualname__, f"{len(raw_images)} Dark frames loaded")

        ## TODO; if the DETLIN file exists, extract the bad pixel map, if not, initialize
        ## a mask set to zeros.
        
        Msg.info(self.__class__.__qualname__, f"Pretending to copy mask from DETLIN product to product")
        
        bpMask = cpl.core.Image.zeros(2048, 2048, cpl.core.Type.INT)

        Msg.info(self.__class__.__qualname__, f"Pretending to do persistence correction")
        
        Msg.info(self.__class__.__qualname__, f"Pretending to correct for non-linearity")

        Msg.info(self.__class__.__qualname__, f"Combining images using method {method!r}")
        combined_image = raw_images.collapse_create()
        print(combined_image.get_median())

        Msg.info(self.__class__.__qualname__, "Pretending to calculate noise")
        
        # simple calculation assuming noise is sqrt of image
        
        noise = cpl.core.Image(combined_image)
        noise.copy_into(combined_image, 0, 0)
        noise.power(0.5)
        
        Msg.info(self.__class__.__qualname__, "Calculate outlying pixels")
        
        darkRms = combined_image.get_stdev()
        darkMedian = combined_image.get_median()
        
        # get masks from thresholds for bad, hot and cold pixels
        # count the number of bad pixels in each, for later
        # change to Image type from mask for later calculations
        
        mask = cpl.core.Mask.threshold_image(combined_image, darkMedian - 2*darkRms, darkMedian + 2*darkRms, 1)
        qcnbad = mask.count()
        mask = cpl.core.Image(mask,dtype = cpl.core.Type.INT)

        maskCold = cpl.core.Mask.threshold_image(combined_image, 0, darkMedian - 2*darkRms, 1)
        qcncold = maskCold.count()
        maskCold = cpl.core.Image(maskCold,dtype = cpl.core.Type.INT)

        maskHot = cpl.core.Mask.threshold_image(combined_image, 0, darkMedian + 2*darkRms, 1)
        qcnhot = maskHot.count()
        maskHot = cpl.core.Image(maskHot,dtype = cpl.core.Type.INT)

        # multiple masks to the correct bitmask
        mask.multiply_scalar(badBit)
        maskCold.multiply_scalar(coldBit)
        maskHot.multiply_scalar(hotBit)

        # and update main mask
        print(type(bpMask))
        bpMask.add(mask)
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
            print(im.get_median())
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
            
        product = self.ProductMasterDark(header, combined_image,)
        
        return {product}


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
