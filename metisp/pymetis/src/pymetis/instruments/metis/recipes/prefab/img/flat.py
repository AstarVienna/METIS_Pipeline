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

from pymetis.drl.noise import estimate_noise_list, calculate_outliers
from pymetis.engine.qc import QcParameterSet
from pymetis.engine.dataitems import DataItem, Hdu, PipelineProductSet

from pymetis.instruments.metis.dataitems.masterflat import MasterImgFlat, FlatRaw
from pymetis.instruments.metis.dataitems.badpixmap import BadPixMap
from pymetis.instruments.metis.inputs import (RawInput, OptionalInputMixin,
                                              PersistenceMapInput, GainMapInput, LinearityInput)
from pymetis.instruments.metis.recipes.base import MetisRecipeImpl
from pymetis.instruments.metis.recipes.prefab.darkimage import DarkImageProcessor

if TYPE_CHECKING:
    from pymetis.engine.recipes.recipe import Recipe
from pymetis.instruments.metis.qc.flat import (MFlatRms, MFlatNbadpix, FlatMean, FlatRms,
                                               FlatMedianMin, FlatMedianMax, FlatMedianRms)


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
        MFlatRms = MFlatRms
        MFlatNBadpix = MFlatNbadpix
        FlatMean = FlatMean
        FlatRms = FlatRms
        FlatMedianMin = FlatMedianMin
        FlatMedianMax = FlatMedianMax
        FlatMedianRms = FlatMedianRms
        
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

        bad_bit = 8

        Msg.info(self.__class__.__qualname__, "Loading flat images")

        self.inputset.raw.load_structure()
        raw_images = self.inputset.raw.load_data('DET1.DATA')

        Msg.info(self.__class__.__qualname__, "Pretending to load DETLIN")

        # TODO add detlin stuff
        
        Msg.info(self.__class__.__qualname__, "Faking a gain map and badpix map")
        Msg.info(self.__class__.__qualname__, f"TTT {type(raw_images[0])}")

        # fake the bp mask by initializing to zero
        badpix_mask = cpl.core.Image.zeros(raw_images[0].width, raw_images[0].height, cpl.core.Type.INT)

        # fake the gain at the moment by setting to 1 TODO real version
        gain = cpl.core.Image.zeros_like(raw_images[0])
        gain.add_scalar(1)

        raw_images = self.correct_gain(raw_images, gain)
        raw_images = self.correct_persistence(raw_images)

        #linearity_map = self.inputset.linearity.load_data(extension=rf'DET{detector:1d}.SCI')
        #raw_images = self.correct_nonlinearity(raw_images, linearity_map)

        # convert the raw images to HDRL image list TODO propogate readnoise somehow

        raw_images_hdrl = estimate_noise_list(raw_images, 0)

        # subtract the darks, now in HDRL format
        # FixMe: the result is never used -- the master flat below is computed from the
        #        *non*-dark-subtracted images (`raw_images_hdrl`). To be resolved with the team.
        _dark_corrected = self.subtract_dark(raw_images_hdrl)

        # FixMe: At skeleton level we just copy the header from the first raw
        primary_header = self.inputset.raw.items[0].primary_header

        # Combine the images in the image list using the image stacking option requested by the user.
        method = self.parameters[f"{self.name}.stacking.method"].value

        # create a static mask that only considers the illuminated portion of the frame
        # set this based on data

        stat_mask = cpl.core.Mask(raw_images_hdrl[0].width, raw_images_hdrl[0].height)
        stat_mask[0:raw_images_hdrl[0].width][0:raw_images_hdrl[0].height] = True
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

        # flag deviant pixels
        # TODO this needs some more thought; if there's any global gradiants in the flat,
        # a simple RMS could exclude real parts of the flat. For now, simple rms,
        # for later, probably a rejection from the local values
        # also, maybe a coverage value? 

        # get hot/cold pixels
        mask_hot, mask_cold = calculate_outliers(mflat, kappa_low=self.kappa_low, kappa_high=self.kappa_high)
        qcnbad  = mask_hot.count() + mask_cold.count()

        Msg.info(self.__class__.__qualname__,
                 f"Updating mask: {qcnbad} outlier pixels masked: ")

        # add the individual masks to the cpl mask
        self.update_mask(badpix_mask, bad_bit, badpix_mask)

        ## copy bad pixel mask to combined_image before calculating QC parameters
        self.apply_mask(mflat, badpix_mask, [1,2,4,8])

        Msg.info(self.__class__.__qualname__, "Calculating QC parameters")

        
        qcrms = mflat.image.get_stdev()

        Msg.info(self.__class__.__qualname__, f"QC FLAT N BADPIX = {qcnbad}")
        Msg.info(self.__class__.__qualname__, f"QC FLAT RMS = {qcrms}")

        medians = []
        # calculate the stats in each individual image
        for im in raw_images:
            # mask bad pixels before calculations
            self.apply_mask(im,badpix_mask,[1,2,4,8])
            medians.append(im.get_median())

        medians=np.array(medians)
        qcmedmin = medians.min()
        qcmedmax = medians.max()
        qcmedrms = medians.std()

        Msg.info(self.__class__.__qualname__, f"QC FLAT MEDIAN MIN = {qcmedmin}")
        Msg.info(self.__class__.__qualname__, f"QC DARK MEDIAN MAX = {qcmedmax}")
        Msg.info(self.__class__.__qualname__, f"QC DARK MEDIAN RMS = {qcmedrms}")

        # now the QC paramters


        self.collect_qc_parameters(
                MFlatRms(qcrms),
                MFlatNbadpix(qcnbad),
                #FltMean(qcmean),  #I'm not sure what these are actually supposed to be; DRLD implies per frame, which would mean N of each
                #FlatRms(qcnbad),  #TODO
                FlatMedianMin(qcmedmin),
                FlatMedianMax(qcmedmax),
                FlatMedianRms(qcmedrms)
            )

        
        header_image = cpl.core.PropertyList()

        header_image = cpl.core.PropertyList.load(self.inputset.raw.frameset[0].file, 0)
        header_noise = copy.deepcopy(header_image)
        header_mask = copy.deepcopy(header_image)


        
        product = self.ProductSet.MasterFlat(
            primary_header,
            Hdu(header_image, mflat.image, name=r'DET1.SCI'),
            Hdu(header_noise, mflat.error, name=r'DET1.ERR'),
            Hdu(header_mask, badpix_mask, name=r'DET1.DQ')
        )

        return {product}
