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
import numpy as np

from typing import Literal, Dict, Any

import cpl
from cpl.core import Msg

from pymetis.drl.combine import combine_images
from pymetis.drl.noise import estimate_noise_list, calculate_outliers
from pymetis.engine.core.classes.image import EnhancedImage
from pymetis.engine.core.parameter import ParameterList, ParameterEnum, ParameterValue

from pymetis.engine.dataitems import DataItem, Hdu, PipelineProductSet
from pymetis.engine.qc import QcParameterSet
from pymetis.engine.recipes import Recipe
from pymetis.drl.image import zeros_like
from pymetis.engine.core.functions.dummy import create_dummy_header
from pymetis.instruments.metis.description import Metis

from pymetis.instruments.metis.recipes.prefab.persistence import PersistenceCorrectionMixin
from pymetis.instruments.metis.dataitems.masterdark.masterdark import MasterDark
from pymetis.instruments.metis.dataitems.masterdark.raw import DarkRaw
from pymetis.instruments.metis.inputs import (RawInput, BadPixMapInput, PersistenceMapInput,
                                              GainMapInput, OptionalInputMixin)
from pymetis.instruments.metis.recipes.base import MetisRecipeImpl
from pymetis.instruments.metis.recipes.prefab import RawImageProcessor

from pymetis.instruments.metis.qc.dark import (DarkMean, DarkMedian, DarkRms, DarkNColdpix, DarkNHotpix, DarkNBadpix,
                                               DarkMedianMedian, DarkMedianMean,
                                               DarkMedianRms, DarkMedianMin, DarkMedianMax)


class MetisDetDarkImpl(PersistenceCorrectionMixin, RawImageProcessor, MetisRecipeImpl):
    """
    Implementation class for the `metis_det_dark` recipe.
    """

    # We start by deriving the implementation class from `MetisRecipeImpl`, or in this case, one of its subclasses,
    # namely `RawImageProcessor, as this recipe processes raw images and we would like to reuse the functionality.

    # First of all, we need to define the input set. Since we are deriving from `RawImageProcessor`,
    # we may reuse the `InputSet` class from it too. This automatically adds a `RawInput` for us.
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
        class GainMapInput(OptionalInputMixin, GainMapInput):
            pass

        # Finally, we bind every input class to an instance attribute. `__init__` creates
        # `self.raw`, `self.persistence_map`, ... as instances of the annotated classes;
        # this is what `process()` accesses via `self.inputset.raw` etc.
        # A class that overrides an input class must also re-annotate the attribute.
        raw: RawInput
        persistence_map: PersistenceMapInput
        bad_pix_map: BadPixMapInput
        gain_map: GainMapInput

        #class LinearityInput(OptionalInputMixin, LinearityInput):
        #    pass


    class ProductSet(PipelineProductSet):
        # Assign product classes. This should be just a data item class.
        # It is not strictly necessary, and we can create the product directly,
        # but it enables us to introspect the class for the manpage and DRLD.
        MasterDark = MasterDark

    class Qc(QcParameterSet):
        DarkMedian = DarkMedian
        DarkMean = DarkMean
        DarkRms = DarkRms
        DarkNBadpix = DarkNBadpix
        DarkNColdpix = DarkNColdpix
        DarkNHotpix = DarkNHotpix
        DarkMedianMean = DarkMedianMean
        DarkMedianMedian = DarkMedianMedian
        DarkMedianRms = DarkMedianRms
        DarkMedianMin = DarkMedianMin
        DarkMedianMax = DarkMedianMax

    # At this point, we should have all inputs and outputs defined -- the "what" part of the recipe implementation.
    # Now we define the "how" part, or the actions to be performed on the data.
    # See the documentation of the parent's `process` function for more details.
    # Feel free to define other functions to break up the algorithm into more manageable chunks
    # and call them from within `process` as needed.


    #############################################################################
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
    #
    # Once 3 extension IO is implemented, rewrite to utilize pyHDRL functionality 
    ###############################################################################

    def __init__(self,
                 recipe: 'Recipe',
                 frameset: cpl.ui.FrameSet,
                 settings: Dict[str, Any]) -> None:
        super().__init__(recipe, frameset, settings)
        self.stacking_method = self.parameters["metis_det_dark.stacking.method"].value
        self.kappa_low = self.parameters["metis_det_dark.outliers.kappa_low"].value
        self.kappa_high = self.parameters["metis_det_dark.outliers.kappa_high"].value

    def _process_single_detector(self, detector: Literal[1, 2, 3, 4]) -> list[Hdu]:
        assert detector in [1, 2, 3, 4], \
            f"Unknown detector {detector}"

        Msg.info(self.__class__.__qualname__,
                 f"Processing detector {detector}")

        raw_images = self.inputset.raw.load_data(extension=f'DET{detector:1d}.DATA')

        # load raw data

        Msg.info(self.__class__.__qualname__, "Pretending to load DETLIN")

        # TODO add detlin stuff
        
        Msg.info(self.__class__.__qualname__, "Faking a gain map and badpix map")

        #TODO optional badpix map

        # fake the gain at the moment by setting to 1 TODO real version
        gain = cpl.core.Image.zeros_like(raw_images[0])
        gain.add_scalar(1)
    
        raw_images = self.correct_gain(raw_images, gain)
        # raw_images = self.correct_persistence(raw_images) # currently fails

        #linearity_map = self.inputset.linearity.load_data(extension=rf'DET{detector:1d}.SCI')
        #raw_images = self.correct_nonlinearity(raw_images, linearity_map)

        if len(raw_images) > 1:
            Msg.info(self.__class__.__qualname__,
                     f"Calculating read noise from {len(raw_images)} raw dark frames")
            diff = cpl.core.Image(raw_images[0])
            diff.subtract(raw_images[1])
            read_noise = cpl.drs.detector.get_noise_window(diff, None)
        else:
            Msg.warning(self.__class__.__qualname__,
                        "Cannot calculate actual read noise as there is only one raw image")
            read_noise = (0, 0)

        # turn the raw images into HDRL images with an initial noise estimate
        raw_images_hdrl = estimate_noise_list(raw_images, read_noise[0])

        combined_image = combine_images(raw_images_hdrl, self.stacking_method)

        # EI version: create in place, along with an empty mask
        output = EnhancedImage.from_hdrl(
            combined_image,
            zeros_like(raw_images[0], cpl.core.Type.INT),
            prefix=f'DET{detector:1d}',
        )

        Msg.info(self.__class__.__qualname__, f"Combining images using method {self.stacking_method!r}")

        # get hot/cold pixels
        mask_hot, mask_cold = calculate_outliers(combined_image, kappa_low=self.kappa_low, kappa_high=self.kappa_high)
        qcnhot, qcncold = mask_hot.count(), mask_cold.count()

        output.dq.add(mask_hot, Metis.MaskFlags.HOT)
        output.dq.add(mask_cold, Metis.MaskFlags.COLD)

        # get noisy pixels: we may need to revisit whether this is a good thing to do later TODO
        
        mask_bad = self.calculate_outliers_sequence(raw_images_hdrl, kappa_low=self.kappa_low, kappa_high=self.kappa_high)
        qcnbad = mask_bad.count()

        Msg.info(self.__class__.__qualname__,
                 f"Updating mask: {(mask_cold | mask_hot | mask_bad).count()} pixels masked: "
                 f"{qcnbad} bad + {qcnhot} hot + {qcncold} cold")

        output.dq.add(mask_bad, Metis.MaskFlags.BAD)
        # The stacking itself may have rejected pixels (depending on the method);
        # record them before `reject` overwrites the scratch masks.
        output.dq.add(output.rejected(), Metis.MaskFlags.BAD)
        output.reject()

        # Reject the same pixels on the local combined image, so that the QC
        # statistics are computed from the valid pixels only.
        bad_pixels = output.dq.flatten()
        combined_image.reject_from_mask(bad_pixels)

        Msg.info(self.__class__.__qualname__, "Actually Calculating QC parameters")

        # calculate the stats in each individual image
        medians = []
        means = []
        stdevs = []
        mins = []
        maxs = []
        for im in raw_images:
            # exclude the bad pixels from the per-frame statistics
            im.reject_from_mask(bad_pixels)
            medians.append(im.get_median())
            means.append(im.get_mean())
            stdevs.append(im.get_stdev())
            mins.append(im.get_min())
            maxs.append(im.get_max())

        header_image = cpl.core.PropertyList()

        hh = header_image.load(self.inputset.raw.frameset[0].file, 0)
        Msg.info(self.__class__.__qualname__, "Appending QC Parameters to header")

        gg = self.collect_qc_parameters(
            DarkMean(combined_image.image.get_mean()),
            DarkMedian(combined_image.image.get_median()),
            DarkRms(combined_image.image.get_stdev()),
            DarkNBadpix(qcnbad),
            DarkNColdpix(qcncold),
            DarkNHotpix(qcnhot),
            DarkMedianMean(np.median(np.array(means))),
            DarkMedianMedian(np.median(np.array(medians))),
            DarkMedianRms(np.median(np.array(stdevs))),
            DarkMedianMin(np.median(np.array(mins))),
            DarkMedianMax(np.median(np.array(maxs))),
        )

        header_image.append(gg)
        header_image.append(hh)

        # for the time being append READNOISE to the header
        header_image.append(cpl.core.Property("READNOISE", cpl.core.Type.DOUBLE, read_noise[0]))
        for elem in header_image:
            Msg.info(self.__class__.__qualname__, f"HEADER IMAGE{elem}")

        output.header_image = header_image
        output.header_error = copy.deepcopy(header_image) # FixMe this is temporary
        output.header_dq = copy.deepcopy(header_image)    # FixMe this is temporary

        return output.hdus()


    def process(self) -> set[DataItem]:
        # load calibration files

        # ToDo: preprocessing steps like persistence correction / nonlinearity
        # ToDo: (or not) -- move to RawImageProcessor anyway
        Msg.info(self.__class__.__qualname__, "Loading raw dark data")
        self.inputset.raw.load_structure()

        # ToDo This feels stupid but works with all detector types. Find a more robust way maybe?
        detector_count = len(list(filter(lambda x: re.match(r'DET[0-9].DATA', x) is not None,
                                  self.inputset.raw.items[0].hdus.keys() - ['PRIMARY'])))

        hdus = functools.reduce(operator.add, map(self._process_single_detector, range(1, detector_count + 1)))

        product = self.ProductSet.MasterDark(
            create_dummy_header(),
            *hdus,
        )
        return {product}


# This is the actual recipe class that is visible by `pyesorex`.
class MetisDetDark(Recipe):
    # Fill in recipe information for `pyesorex`. These are required and checked by `pyesorex`.
    _name = "metis_det_dark"
    _version = "0.1"
    _author = "Hugo Buddelmeijer, A*"
    _email = "hugo@buddelmeijer.nl"
    _synopsis = "Create master dark"
    _description = (
        "Prototype to create a METIS masterdark for {detector} in {2RG, GEO, IFU}"
    )

    # And also fill in information from DRLD. These are specific to METIS and are used to build the description
    # for the man page. Later, we would like to be able to compare them directly to DRLD and test for that.
    _matched_keywords: frozenset[str] = frozenset()
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
            alternatives=("average", "median", "sigclip"),
        ),
        ParameterValue(
            name=f"{_name}.outliers.kappa_low",
            context=_name,
            description="Lower bound for bad pixel clipping, in standard deviations",
            default=5,
        ),
        ParameterValue(
            name=f"{_name}.outliers.kappa_high",
            context=_name,
            description="Upper bound for bad pixel clipping, in standard deviations",
            default=5,
        ),
    ])

    # Point the `implementation_class` to the *top* class of your recipe hierarchy.
    # All promotions should happen at instantiation time.
    Impl = MetisDetDarkImpl
