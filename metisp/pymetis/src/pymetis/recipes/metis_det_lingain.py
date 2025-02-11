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

import re

from abc import ABC
from typing import Dict

import cpl

from pymetis.base.recipe import MetisRecipe
from pymetis.base.product import PipelineProduct, DetectorSpecificProduct
from pymetis.inputs.common import RawInput, BadpixMapInput
from pymetis.mixins.detector import Detector2rgMixin, DetectorGeoMixin, DetectorIfuMixin
from pymetis.prefab.rawimage import RawImageProcessor


class LinGainProduct(DetectorSpecificProduct, ABC):
    """ Common base class for all linearity and gain products. Just sets `group`, `level` and `frame_type`. """
    _group = cpl.ui.Frame.FrameGroup.PRODUCT
    _level = cpl.ui.Frame.FrameLevel.FINAL
    _frame_type = cpl.ui.Frame.FrameType.IMAGE


class MetisDetLinGainImpl(RawImageProcessor):
    class InputSet(RawImageProcessor.InputSet):
        class RawInput(RawInput):
            _tags = re.compile(r"DETLIN_(?P<detector>2RG|GEO|IFU)_RAW")

        class WcuOffInput(RawInput):
            _title = "WCU off raw"
            _tags = re.compile(r"(?P<band>LM|N|IFU)_WCU_OFF_RAW")

        def __init__(self, frameset: cpl.ui.FrameSet):
            super().__init__(frameset)
            self.wcu_off = self.WcuOffInput(frameset)
            self.badpix_map = BadpixMapInput(frameset, required=False)
            self.inputs |= {self.badpix_map, self.wcu_off}

    class ProductGain(LinGainProduct):
        @property
        def tag(self) -> str:
            return f"GAIN_MAP_{self.detector:s}"

    class ProductLinearity(LinGainProduct):
        @property
        def tag(self) -> str:
            return f"LINEARITY_{self.detector:s}"

    class ProductBadpixMap(LinGainProduct):
        @property
        def tag(self) -> str:
            return f"BADPIX_MAP_{self.detector:s}"

    def process_images(self) -> [PipelineProduct]:
        raw_images = self.inputset.load_raw_images()
        combined_image = self.combine_images(raw_images,
                                             method=self.parameters["metis_det_lingain.stacking.method"].value)

        # Flat field preparation: subtract bias and normalize it to median 1
        # Msg.info(self.name, "Preparing flat field")
        # if flat_image:
        #     if bias_image:
        #         flat_image.subtract(bias_image)
        #     median = flat_image.get_median()
        #     flat_image.divide_scalar(median)
        header = cpl.core.PropertyList.load(self.inputset.raw.frameset[0].file, 0)

        gain_image = combined_image         # TODO Actual implementation missing
        linearity_image = combined_image    # TODO Actual implementation missing
        badpix_map = combined_image         # TODO Actual implementation missing

        product_gain_map = self.ProductGain(self, header, gain_image, detector=self.detector)
        product_linearity = self.ProductLinearity(self, header, linearity_image, detector=self.detector)
        product_badpix_map = self.ProductBadpixMap(self, header, badpix_map, detector=self.detector)

        return [product_gain_map, product_linearity, product_badpix_map]


class Metis2rgLinGainImpl(Detector2rgMixin, MetisDetLinGainImpl):
    pass


class MetisGeoLinGainImpl(DetectorGeoMixin, MetisDetLinGainImpl):
    pass


class MetisIfuLinGainImpl(DetectorIfuMixin, MetisDetLinGainImpl):
    pass


class MetisDetLinGain(MetisRecipe):
    # Fill in recipe information
    _name = "metis_det_lingain"
    _version = "0.1"
    _author = "A*Vienna"
    _email = "hugo@buddelmeijer.nl"
    _synopsis = "Measure detector non-linearity and gain"
    _description = (
        "Prototype to create a METIS linear gain map."
    )

    parameters = cpl.ui.ParameterList([
        cpl.ui.ParameterEnum(
            name=rf"{_name}.stacking.method",
            context=_name,
            description="Name of the method used to combine the input images",
            default="median",
            alternatives=("add", "average", "median"),
        ),
        cpl.ui.ParameterValue(
            name=rf"{_name}.threshold.lowlim",
            context=_name,
            description="Thresholding threshold lower limit",
            default=0,
        ),
        cpl.ui.ParameterValue(
            name=rf"{_name}.threshold.uplim",
            context=_name,
            description="Thresholding threshold upper limit",
            default=0,
        ),
    ])

    implementation_class = MetisDetLinGainImpl

    def dispatch_implementation_class(self, frameset):
        input = self.implementation_class.InputSet.RawInput(frameset)
        return {
            '2RG': Metis2rgLinGainImpl,
            'GEO': MetisGeoLinGainImpl,
            'IFU': MetisIfuLinGainImpl,
        }[input.detector]