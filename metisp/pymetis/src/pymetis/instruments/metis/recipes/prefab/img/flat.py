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

from cpl.core import Msg
from typing import Dict, Any, TYPE_CHECKING
import copy
import numpy as np

import hdrl, cpl

from pymetis.drl.image import zeros_like
from pymetis.drl.noise import estimate_noise_list, calculate_outliers
from pymetis.engine.core.classes.image import EnhancedImage
from pymetis.engine.qc import QcParameterSet
from pymetis.engine.dataitems import DataItem, PipelineProductSet

from pymetis.instruments.metis.dataitems.masterflat import MasterImgFlat, FlatRaw
from pymetis.instruments.metis.dataitems.badpixmap import BadPixMap
from pymetis.instruments.metis.description import Metis
from pymetis.instruments.metis.inputs import (RawInput, OptionalInputMixin,
                                              PersistenceMapInput, GainMapInput, LinearityInput)
from pymetis.instruments.metis.recipes.base import MetisRecipeImpl
from pymetis.instruments.metis.recipes.prefab.darkimage import DarkImageProcessor
from pymetis.instruments.metis import qc

if TYPE_CHECKING:
    from pymetis.engine.recipes.recipe import Recipe


class MetisBaseImgFlatImpl(DarkImageProcessor, MetisRecipeImpl, ABC):
    class InputSet(DarkImageProcessor.InputSet):
        """
        Base class for Inputs which create flats. Requires a set of raw frames and a master dark.
        """
        class PersistenceMapInput(OptionalInputMixin, PersistenceMapInput):
            pass

        class RawInput(RawInput):
            Item = FlatRaw

        persistence_map: PersistenceMapInput
        gain_map: GainMapInput
        linearity: LinearityInput
        raw: RawInput

    class ProductSet(PipelineProductSet):
        MasterFlat = MasterImgFlat
        BadPixMap = BadPixMap

    class Qc(QcParameterSet):
        MFlatRms = qc.flat.MFlatRms
        MFlatNBadpix = qc.flat.MFlatNBadpix
        FlatMean = qc.flat.FlatMean
        FlatRms = qc.flat.FlatRms
        FlatMedianMin = qc.flat.FlatMedianMin
        FlatMedianMax = qc.flat.FlatMedianMax
        FlatMedianRms = qc.flat.FlatMedianRms
        
    def __init__(self,
                 recipe: 'Recipe',
                 frameset: cpl.ui.FrameSet,
                 settings: Dict[str, Any]) -> None:
        super().__init__(recipe, frameset, settings)
        self.stacking_method = self.parameters[f"{self.name}.stacking.method"].value
        self.kappa_low = self.parameters[f"{self.name}.outliers.kappa_low"].value
        self.kappa_high = self.parameters[f"{self.name}.outliers.kappa_high"].value

    def process(self) -> set[DataItem]:
        """
        Do the actual processing of the images.
        Here, it means loading the input images and a master dark,
        then subtracting the master dark from every flat,
        and finally combining them into a master flat, using the HDRL flat methods.
        """
        # TODO: Detect detector
        # TODO: Lamp

        # target = self.inputset.tag_parameters['target']

        Msg.info(self.__class__.__qualname__, "Loading flat images")

        self.inputset.raw.load_structure()
        raw_images = self.inputset.raw.load_data('DET1.DATA')

        Msg.info(self.__class__.__qualname__, "Pretending to load DETLIN")

        # TODO add detlin stuff

        Msg.info(self.__class__.__qualname__, "Faking a gain map")

        # fake the gain at the moment by setting to 1 TODO real version
        gain = cpl.core.Image.zeros_like(raw_images[0])
        gain.add_scalar(1)

        raw_images = self.correct_gain(raw_images, gain)
        raw_images = self.correct_persistence(raw_images)

        #linearity_map = self.inputset.linearity.load_data(extension=rf'DET{detector:1d}.SCI')
        #raw_images = self.correct_nonlinearity(raw_images, linearity_map)

        # convert the raw images to HDRL image list TODO propogate readnoise somehow

        raw_images_hdrl = estimate_noise_list(raw_images, 0)

        # Subtract the master dark. `subtract_dark` works on the list in place and
        # returns it, so the master flat below is computed from dark-subtracted frames.
        raw_images_hdrl = self.subtract_dark(raw_images_hdrl)

        # FixMe: At skeleton level we just copy the header from the first raw
        primary_header = self.inputset.raw.items[0].primary_header

        # Combine the images in the image list using the image stacking option requested by the user.
        method = self.parameters[f"{self.name}.stacking.method"].value

        # A static mask restricting the statistics to the illuminated portion of
        # the frame is not used yet; it should eventually be derived from the data.
        stat_mask = None

        # create a method paramter for HDRL

        if(method == "median"):
            collapse = hdrl.func.Collapse.Median()
        elif(method == "average"):
            collapse = hdrl.func.Collapse.Mean()
        else:
            Msg.error(self.__class__.__qualname__,
                      f"Got unknown combination method {method!r}. Stopping right here!")
            raise ValueError(f"Unknown combination method {method!r}")

        # create the hdrl.func.Flat instance
        flat = hdrl.func.Flat(1, 1, hdrl.func.Flat.Mode.FreqHigh)

        # compute the master flat with imglist being an hdrl.core.ImageList
        # holding the images to combine into the master flat

        results = flat.compute(raw_images_hdrl, collapse, stat_mask)
        mflat = results.master

        # `mflat` stays the local working copy for the statistics below; `output`
        # carries the data quality layer that is saved with the product.
        output = EnhancedImage.from_hdrl(
            mflat,
            zeros_like(raw_images[0], cpl.core.Type.INT),
            prefix='DET1',
        )

        # flag deviant pixels
        # TODO this needs some more thought; if there's any global gradiants in the flat,
        # a simple RMS could exclude real parts of the flat. For now, simple rms,
        # for later, probably a rejection from the local values
        # also, maybe a coverage value?

        # get hot/cold pixels
        mask_hot, mask_cold = calculate_outliers(mflat, kappa_low=self.kappa_low, kappa_high=self.kappa_high)
        qcnbad = mask_hot.count() + mask_cold.count()

        Msg.info(self.__class__.__qualname__,
                 f"Updating mask: {qcnbad} outlier pixels masked")

        output.dq.add(mask_hot, Metis.MaskFlags.HOT)
        output.dq.add(mask_cold, Metis.MaskFlags.COLD)
        # The flat combination may have rejected pixels of its own; record them
        # before `reject` overwrites the scratch masks.
        output.dq.add(output.rejected(), Metis.MaskFlags.BAD)
        output.reject()

        # Reject the same pixels on the local master flat, so that the QC
        # statistics are computed from the valid pixels only.
        bad_pixels = output.dq.flatten()
        mflat.reject_from_mask(bad_pixels)

        Msg.info(self.__class__.__qualname__, "Calculating QC parameters")

        qcrms = mflat.image.get_stdev()

        Msg.info(self.__class__.__qualname__, f"QC FLAT N BADPIX = {qcnbad}")
        Msg.info(self.__class__.__qualname__, f"QC FLAT RMS = {qcrms}")

        medians = []
        # calculate the stats in each individual raw frame (these are not dark-subtracted)
        for im in raw_images:
            im.reject_from_mask(bad_pixels)
            medians.append(im.get_median())

        medians = np.array(medians)
        qcmedmin = medians.min()
        qcmedmax = medians.max()
        qcmedrms = medians.std()

        Msg.info(self.__class__.__qualname__, f"QC FLAT MEDIAN MIN = {qcmedmin}")
        Msg.info(self.__class__.__qualname__, f"QC FLAT MEDIAN MAX = {qcmedmax}")
        Msg.info(self.__class__.__qualname__, f"QC FLAT MEDIAN RMS = {qcmedrms}")

        qc = self.collect_qc_parameters(
            self.Qc.MFlatRms(qcrms),
            self.Qc.MFlatNBadpix(qcnbad),
            #self.Qc.FltMean(qcmean),  #I'm not sure what these are actually supposed to be; DRLD implies per frame, which would mean N of each
            #self.Qc.FlatRms(qcnbad),  #TODO
            self.Qc.FlatMedianMin(qcmedmin),
            self.Qc.FlatMedianMax(qcmedmax),
            self.Qc.FlatMedianRms(qcmedrms)
        )

        # FixMe: At skeleton level the primary header of the first raw serves as the
        #        extension header, for all three layers
        header_image = cpl.core.PropertyList.load(self.inputset.raw.frameset[0].file, 0)
        header_image.append(qc)
        output.header_image = header_image
        output.header_error = copy.deepcopy(header_image)   # FixMe this is temporary
        output.header_dq = copy.deepcopy(header_image)      # FixMe this is temporary

        product = self.ProductSet.MasterFlat(primary_header, *output.hdus())

        return {product}
