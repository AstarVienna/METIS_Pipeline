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

from pymetis.base import MetisRecipe
from pymetis.base.product import PipelineProduct, DetectorSpecificProduct
from pymetis.inputs.common import RawInput
from pymetis.mixins.detector import Detector2rgMixin, DetectorGeoMixin, DetectorIfuMixin
from pymetis.prefab.rawimage import RawImageProcessor
from pymetis.prefab.darkimage import DarkImageProcessor


class LinGainProduct(DetectorSpecificProduct, ABC):
    """ Common base class for all linearity and gain products. Just sets `group`, `level` and `frame_type`. """
    group = cpl.ui.Frame.FrameGroup.PRODUCT
    level = cpl.ui.Frame.FrameLevel.FINAL
    frame_type = cpl.ui.Frame.FrameType.IMAGE


class MetisDetLinGainImpl(DarkImageProcessor):
    class InputSet(RawImageProcessor.InputSet):
        class RawInput(RawInput):
            _tags = re.compile(r"DETLIN_(?P<detector>2RG|GEO|IFU)_RAW")

    class ProductGain(LinGainProduct):
        @property
        def category(self) -> str:
            return f"GAIN_MAP_{self.detector:s}"

    class ProductLinearity(LinGainProduct):
        @property
        def category(self) -> str:
            return f"LINEARITY_{self.detector:s}"

    class ProductBadpixMap(LinGainProduct):
        @property
        def category(self) -> str:
            return f"BADPIX_MAP_{self.detector:s}"

    def process_images(self) -> Dict[str, PipelineProduct]:
        raw_images = self.load_raw_images()
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

        self.products = {
            f'MASTER_GAIN_{self.detector}':
                self.ProductGain(self, header, gain_image,
                                 detector=self.detector),
            f'LINEARITY_{self.detector}':
                self.ProductLinearity(self, header, linearity_image,
                                      detector=self.detector),
            f'BADPIX_MAP_{self.detector}':
                self.ProductBadpixMap(self, header, badpix_map,
                                      detector=self.detector),
        }

        return self.products


class Metis2rgLinGainImpl(Detector2rgMixin, MetisDetLinGainImpl):
    class InputSet(MetisDetLinGainImpl.InputSet):
        class RawInput(MetisDetLinGainImpl.InputSet.RawInput):
            _tags = re.compile(r"DETLIN_2RG_RAW")


class MetisGeoLinGainImpl(DetectorGeoMixin, MetisDetLinGainImpl):
    class InputSet(MetisDetLinGainImpl.InputSet):
        class RawInput(MetisDetLinGainImpl.InputSet.RawInput):
            _tags = re.compile(r"DETLIN_GEO_RAW")


class MetisIfuLinGainImpl(DetectorIfuMixin, MetisDetLinGainImpl):
    class InputSet(MetisDetLinGainImpl.InputSet):
        class RawInput(MetisDetLinGainImpl.InputSet.RawInput):
            _tags = re.compile(r"DETLIN_IFU_RAW")


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