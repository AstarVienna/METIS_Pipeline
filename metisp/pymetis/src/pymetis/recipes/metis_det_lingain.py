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

import functools
import operator
import re

from abc import ABC
from typing import Literal, Dict, Any

import cpl
from cpl.core import Msg
from pyesorex.parameter import ParameterList, ParameterEnum, ParameterValue

from pymetis.classes.dataitems import DataItem
from pymetis.classes.dataitems.hdu import Hdu
from pymetis.dataitems.badpixmap import BadPixMap
from pymetis.dataitems.gainmap import GainMap
from pymetis.dataitems.linearity.linearity import LinearityMap
from pymetis.dataitems.linearity.raw import LinearityRaw
from pymetis.classes.inputs import RawInput, BadPixMapInput, OptionalInputMixin
from pymetis.classes.inputs.common import WcuOffInput
from pymetis.classes.prefab import RawImageProcessor
from pymetis.classes.recipes import MetisRecipe
from pymetis.utils.dummy import create_dummy_header


class MetisDetLinGainImpl(RawImageProcessor, ABC):
    class InputSet(RawImageProcessor.InputSet):
        class RawInput(RawInput):
            Item = LinearityRaw

        WcuOffInput = WcuOffInput

        class BadPixMapInput(OptionalInputMixin, BadPixMapInput):
            Item = BadPixMap

    ProductGainMap = GainMap
    ProductLinearity = LinearityMap
    ProductBadPixMap = BadPixMap

    def __init__(self,
                 recipe: 'MetisRecipe',
                 frameset: cpl.ui.FrameSet,
                 settings: Dict[str, Any]) -> None:
        super().__init__(recipe, frameset, settings)
        self.stacking_method = self.parameters["metis_det_lingain.stacking.method"].value

    def _process_single_detector(self, detector: Literal[1, 2, 3, 4]) -> dict[str, Hdu]:
        det = rf'DET{detector:1d}'

        raw_images = self.inputset.raw.load_data(rf'{det}.DATA')
        combined_image = self.combine_images(raw_images, method=self.stacking_method)

        header_linearity = cpl.core.PropertyList.load(self.inputset.raw.frameset[0].file, 0)
        header_gain = cpl.core.PropertyList.load(self.inputset.raw.frameset[0].file, 0)
        header_badpix = cpl.core.PropertyList.load(self.inputset.raw.frameset[0].file, 0)

        gain_image = combined_image         # TODO Actual implementation missing
        linearity_image = combined_image    # TODO Actual implementation missing
        badpix_map = combined_image         # TODO Actual implementation missing

        return {
            'gain_map': Hdu(header_gain, gain_image, name=rf'{det}.SCI'),
            'linearity_map': Hdu(header_linearity, linearity_image, name=rf'{det}.SCI'),
            'badpix_map': Hdu(header_badpix, badpix_map, name=rf'{det}.SCI'),
            #'gain_map': Hdu(header_gain, gain_image, name='PRIMARY'),
            #'linearity_map': Hdu(header_linearity, linearity_image, name='PRIMARY'),
            #'badpix_map': Hdu(header_badpix, badpix_map, name='PRIMARY'),
        }

    def process(self) -> set[DataItem]:
        Msg.info(self.__class__.__qualname__, f"Loading raw data")
        self.inputset.raw.load_structure()

        # Flat field preparation: subtract bias and normalize it to median 1
        # Msg.info(self.name, "Preparing flat field")
        # if flat_image:
        #     if bias_image:
        #         flat_image.subtract(bias_image)
        #     median = flat_image.get_median()
        #     flat_image.divide_scalar(median)

        Msg.info(self.__class__.__qualname__, f"HDUs: {self.inputset.raw.items[0].hdus.keys()}")

        detector_count = len(list(filter(lambda x: re.match(r'DET[0-9].DATA', x) is not None,
                                         self.inputset.raw.items[0].hdus.keys() - ['PRIMARY'])))
        primary_header = cpl.core.PropertyList.load(self.inputset.raw.frameset[0].file, 0)

        all_hdus = [self._process_single_detector(detector) for detector in range(1, detector_count + 1)]

        product_gain_map = self.ProductGainMap(primary_header, *[output['gain_map'] for output in all_hdus])
        product_linearity = self.ProductLinearity(primary_header, *[output['linearity_map'] for output in all_hdus])
        product_badpix_map = self.ProductBadPixMap(primary_header, *[output['badpix_map'] for output in all_hdus])

        return {product_gain_map, product_linearity, product_badpix_map}


class MetisDetLinGain(MetisRecipe):
    # Fill in recipe information
    _name = "metis_det_lingain"
    _version = "0.1"
    _author = "A*"
    _email = "hugo@buddelmeijer.nl"
    _synopsis = "Measure detector non-linearity and gain"
    _description = (
        "Prototype to create a METIS linear gain map."
    )

    _algorithm = """Subtract instrument dark (hdrl_imagelist_sub_image).
    Compute mean and variance for each frame.
    Gain is determined as the slope of variance against mean (metis_derive_gain).
    Fit polynomial of value as a function of DIT and illumination level for each pixel (metis_derive_nonlinearity).
    Flag pixels with coefficients significantly different from the mean of all pixels (hdrl_bpm_fit_compute)."""

    parameters = ParameterList([
        ParameterEnum(
            name=rf"{_name}.stacking.method",
            context=_name,
            description="Name of the method used to combine the input images",
            default="median",
            alternatives=("add", "average", "median"),
        ),
        ParameterValue(
            name=rf"{_name}.threshold.lowlim",
            context=_name,
            description="Thresholding threshold lower limit",
            default=0,
        ),
        ParameterValue(
            name=rf"{_name}.threshold.uplim",
            context=_name,
            description="Thresholding threshold upper limit",
            default=0,
        ),
    ])

    Impl = MetisDetLinGainImpl
