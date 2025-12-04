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
import copy
import functools
import operator
import re
from abc import ABC
from typing import Self, Optional, Literal, Dict, Any

import cpl
from cpl.core import Msg, ImageList, Image, Mask
from pyesorex.parameter import ParameterList, ParameterEnum, ParameterValue

from pymetis.classes.dataitems import DataItem
from pymetis.classes.dataitems.hdu import Hdu
from pymetis.dataitems.masterdark.masterdark import MasterDark
from pymetis.dataitems.masterdark.raw import DarkRaw
from pymetis.classes.inputs import (RawInput, BadPixMapInput, PersistenceMapInput,
                                    LinearityInput, GainMapInput, OptionalInputMixin)
from pymetis.classes.prefab import RawImageProcessor
from pymetis.classes.recipes import MetisRecipe

from pymetis.qc.dark import DarkMean, DarkMedian, DarkRms, DarkNColdpix, DarkNHotpix, DarkNBadpix, DarkMedianMedian, \
    DarkMedianMean, DarkMedianRms, DarkMedianMin, DarkMedianMax

import numpy as np

from pymetis.functions.image import zeros_like
from pymetis.utils.dummy import create_dummy_header, python_to_cpl_type


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

    # Assign product classes. This should be just a data item class.
    # It is not strictly necessary, and we can create the product directly,
    # but it enables us to introspect the class for the manpage and DRLD.
    ProductMasterDark = MasterDark

    QcDarkMedian = DarkMedian
    QcDarkMean = DarkMean
    QcDarkRms = DarkRms
    QcDarkNBadpix = DarkNBadpix
    QcDarkNColdpix = DarkNColdpix
    QcDarkNHotpix = DarkNHotpix
    QcDarkMedianMean = DarkMedianMean
    QcDarkMedianMedian = DarkMedianMedian
    QcDarkMedianRms = DarkMedianRms
    QcDarkMedianMin = DarkMedianMin
    QcDarkMedianMax = DarkMedianMax

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

    def __init__(self,
            recipe: 'MetisRecipe',
            frameset: cpl.ui.FrameSet,
            settings: Dict[str, Any]) -> None:
        super().__init__(recipe, frameset, settings)
        self.stacking_method = self.parameters["metis_det_dark.stacking.method"].value


    def _process_single_detector(self, detector: Literal[1, 2, 3, 4]) -> list[Hdu]:
        assert detector in [1, 2, 3, 4], \
            f"Unknown detector {detector}"

        Msg.info(self.__class__.__qualname__,
                 f"Processing detector {detector}")

        raw_images = self.inputset.raw.load_data(extension=f'DET{detector:1d}.DATA')

        # load raw data
        kappa_high = 2  # ToDo This could probably be a recipe parameter
        kappa_low = 2   # ToDo This too

        bad_bit = 1
        cold_bit = 1
        hot_bit = 1

        Msg.info(self.__class__.__qualname__, f"Pretending to load DETLIN")

        Msg.info(self.__class__.__qualname__, f"Faking a gain map and badpix map")

        # fake the bp mask by initializing to zero
        badpix_mask = zeros_like(raw_images[0], cpl.core.Type.FLOAT)

        # fake the gain at the moment by setting to 1

        raw_images = self.correct_gain(raw_images)
        raw_images = self.correct_persistence(raw_images)

        linearity_map = self.inputset.linearity.load_data(extension=rf'DET{detector:1d}.SCI')
        raw_images = self.correct_nonlinearity(raw_images, linearity_map)

        if len(raw_images) > 1:
            Msg.info(self.__class__.__qualname__,
                     f"Calculating read noise from {len(raw_images)} raw dark frames")
            diff = cpl.core.Image(raw_images[0])
            diff.subtract(raw_images[1])
            read_noise = cpl.drs.detector.get_noise_window(diff, None)
        else:
            Msg.warning(self.__class__.__qualname__,
                        f"Cannot calculate actual read noise as there is only one raw image")
            read_noise = (0, 0)

        combined_image, noise = self.combine_images_with_error(raw_images, self.stacking_method, read_noise[0])

        Msg.info(self.__class__.__qualname__, f"Combining images using method {self.stacking_method!r}")

        mask_hot, mask_cold = self.calculate_outliers(combined_image, kappa_low=kappa_low, kappa_high=kappa_high)
        qcnhot, qcncold = mask_hot.count(), mask_cold.count()
        mask_bad = self.metis_bpm_3d_compute(raw_images, kappa_low=kappa_low, kappa_high=kappa_high)
        qcnbad = mask_bad.count()

        Msg.info(self.__class__.__qualname__,
                 f"Updating mask: {(mask_cold | mask_hot | mask_bad).count()} pixels masked: "
                 f"{qcnbad} bad + {qcnhot} hot + {qcncold} cold")
        mask_hot = cpl.core.Image(mask_hot, dtype=cpl.core.Type.INT)
        mask_cold = cpl.core.Image(mask_cold, dtype=cpl.core.Type.INT)
        mask_bad = cpl.core.Image(mask_bad, dtype=cpl.core.Type.INT)


        # multiple masks to the correct bitmask
        # ToDo [Martin] What does this do? Multiply by one?
        mask_bad.multiply_scalar(bad_bit)
        mask_cold.multiply_scalar(cold_bit)
        mask_hot.multiply_scalar(hot_bit)

        # and update main mask
        badpix_mask.add(mask_bad)
        badpix_mask.add(mask_hot)
        badpix_mask.add(mask_cold)

        ## how to copy mask into image?

        Msg.info(self.__class__.__qualname__, "Actually Calculating QC parameters")

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

        header_image = cpl.core.PropertyList.load(self.inputset.raw.frameset[0].file, 0)
        Msg.info(self.__class__.__qualname__, "Appending QC Parameters to header")

        header_image.append(
            self.collect_qc_parameters(
                DarkMean(qcmean),
                DarkMedian(qcmed),
                DarkRms(qcrms),
                DarkNBadpix(qcnbad),
                DarkNColdpix(qcncold),
                DarkNHotpix(qcnhot),
                DarkMedianMean(qcmedmean),
                DarkMedianMedian(qcmedmed),
                DarkMedianRms(qcmedrms),
                DarkMedianMin(qcmedmin),
                DarkMedianMax(qcmedmax),
            )
        )

        header_noise = copy.deepcopy(header_image)
        header_mask = copy.deepcopy(header_image)

        return [
            Hdu(header_image, combined_image, name=rf'DET{detector:1d}.SCI'),
            Hdu(header_noise, noise, name=rf'DET{detector:1d}.ERR'),
            Hdu(header_mask, badpix_mask, name=rf'DET{detector:1d}.DQ'),
        ]

    def process(self) -> set[DataItem]:
        # load calibration files

        # ToDo: preprocessing steps like persistence correction / nonlinearity
        # ToDo: (or not) -- move to RawImageProcessor anyway
        Msg.info(self.__class__.__qualname__, f"Loading raw dark data")
        self.inputset.raw.load_structure()

        # ToDo This feels stupid but works with all detector types. Find a more robust way maybe?
        detector_count = len(list(filter(lambda x: re.match(r'DET[0-9].DATA', x) is not None,
                                  self.inputset.raw.items[0].hdus.keys() - ['PRIMARY'])))

        hdus = functools.reduce(operator.add, map(self._process_single_detector, range(1, detector_count + 1)))

        product = self.ProductMasterDark(
            create_dummy_header(),
            *hdus,
        )
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
        # ToDo: Maybe these should be user-configurable as well?
        #ParameterValue(
        #    name=f"{_name}.outliers.kappa_low",
        #    context=_name,
        #    description="Lower bound for bad pixel clipping, in standard deviations",
        #    default=2,
        #),
        #ParameterValue(
        #    name=f"{_name}.outliers.kappa_high",
        #    context=_name,
        #    description="Upper bound for bad pixel clipping, in standard deviations",
        #    default=2,
        #),
    ])

    # Point the `implementation_class` to the *top* class of your recipe hierarchy.
    # All promotions should happen at instantiation time.
    Impl = MetisDetDarkImpl
