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

import cpl

from pyesorex.parameter import ParameterList, ParameterEnum, ParameterValue

from pymetis.classes.dataitems.badpixmap import BadPixMap, BadPixMap2rg, BadPixMapGeo, BadPixMapIfu
from pymetis.classes.dataitems.common import LinearityMap
from pymetis.classes.dataitems.dataitem import DataItem
from pymetis.classes.dataitems.gainmap import GainMap, GainMap2rg, GainMapGeo, GainMapIfu
from pymetis.classes.dataitems.linearity.linearity import Linearity2rg, LinearityGeo, LinearityIfu
from pymetis.classes.dataitems.linearity.raw import Linearity2rgRaw, LinearityRaw, LinearityGeoRaw, LinearityIfuRaw
from pymetis.classes.dataitems.raw.wcuoff import WcuOffRaw
from pymetis.classes.mixins.detector import Detector2rgMixin, DetectorGeoMixin, DetectorIfuMixin
from pymetis.classes.recipes import MetisRecipe
from pymetis.classes.prefab import RawImageProcessor
from pymetis.classes.inputs import RawInput, BadpixMapInput, OptionalInputMixin
from pymetis.classes.products import PipelineProduct, PipelineImageProduct
from pymetis.classes.products import DetectorSpecificProduct


class LinGainProduct(DetectorSpecificProduct, PipelineImageProduct, ABC):
    """ Common base class for all linearity and gain products. It just sets `level`. """
    level = cpl.ui.Frame.FrameLevel.FINAL


class MetisDetLinGainImpl(RawImageProcessor, ABC):
    class InputSet(RawImageProcessor.InputSet):
        class RawInput(RawInput):
            _item = LinearityRaw
            _tags: re.Pattern = re.compile(r"DETLIN_(?P<detector>2RG|GEO|IFU)_RAW")
            _description: str = "Raw data for non-linearity determination."

        class WcuOffInput(RawInput):
            _item = WcuOffRaw
            _tags: re.Pattern = re.compile(r"(?P<band>LM|N|IFU)_WCU_OFF_RAW")

        class BadpixMapInput(OptionalInputMixin, BadpixMapInput):
            _item = BadPixMap
            _required: bool = False

    class ProductGain(LinGainProduct):
        _item = GainMap

    class ProductLinearity(LinGainProduct):
        _item = LinearityMap

    class ProductBadpixMap(LinGainProduct):
        _item = BadPixMap

    def process_images(self) -> set[PipelineProduct]:
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

        product_gain_map = self.ProductGain(self, header, gain_image)
        product_linearity = self.ProductLinearity(self, header, linearity_image)
        product_badpix_map = self.ProductBadpixMap(self, header, badpix_map)

        return {product_gain_map, product_linearity, product_badpix_map}

    def _dispatch_child_class(self) -> type["MetisDetLinGainImpl"]:
        return {
            '2RG': Metis2rgLinGainImpl,
            'GEO': MetisGeoLinGainImpl,
            'IFU': MetisIfuLinGainImpl,
        }[self.inputset.detector]


class Metis2rgLinGainImpl(MetisDetLinGainImpl):
    class InputSet(MetisDetLinGainImpl.InputSet):
        class RawInput(MetisDetLinGainImpl.InputSet.RawInput):
            _item = Linearity2rgRaw

    class ProductGain(Detector2rgMixin, MetisDetLinGainImpl.ProductGain):
        _item = GainMap2rg

    class ProductLinearity(Detector2rgMixin, MetisDetLinGainImpl.ProductLinearity):
        _item = Linearity2rg

    class ProductBadpixMap(Detector2rgMixin, MetisDetLinGainImpl.ProductBadpixMap):
        _item = BadPixMap2rg


class MetisGeoLinGainImpl(MetisDetLinGainImpl):
    class InputSet(MetisDetLinGainImpl.InputSet):
        class RawInput(MetisDetLinGainImpl.InputSet.RawInput):
            _item = LinearityGeoRaw

    class ProductGain(DetectorGeoMixin, MetisDetLinGainImpl.ProductGain):
        _item = GainMapGeo

    class ProductLinearity(DetectorGeoMixin, MetisDetLinGainImpl.ProductLinearity):
        _item = LinearityGeo

    class ProductBadpixMap(DetectorGeoMixin, MetisDetLinGainImpl.ProductBadpixMap):
        _item = BadPixMapGeo


class MetisIfuLinGainImpl(MetisDetLinGainImpl):
    class InputSet(MetisDetLinGainImpl.InputSet):
        class RawInput(MetisDetLinGainImpl.InputSet.RawInput):
            _item = LinearityIfuRaw

    class ProductGain(DetectorIfuMixin, MetisDetLinGainImpl.ProductGain):
        _item = GainMapIfu

    class ProductLinearity(DetectorIfuMixin, MetisDetLinGainImpl.ProductLinearity):
        _item = LinearityIfu

    class ProductBadpixMap(DetectorIfuMixin, MetisDetLinGainImpl.ProductBadpixMap):
        _item = BadPixMapIfu


class MetisDetLinGain(MetisRecipe):
    # Fill in recipe information
    _name: str = "metis_det_lingain"
    _version: str = "0.1"
    _author: str = "A*"
    _email: str = "hugo@buddelmeijer.nl"
    _synopsis: str = "Measure detector non-linearity and gain"
    _description: str = (
        "Prototype to create a METIS linear gain map."
    )

    _matched_keywords: set[str] = set()
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

    implementation_class = MetisDetLinGainImpl
