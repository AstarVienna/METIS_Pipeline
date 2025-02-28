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

from pymetis.classes.recipes import MetisRecipe
from pymetis.classes.prefab import RawImageProcessor
from pymetis.classes.inputs import RawInput, BadpixMapInput, OptionalInputMixin
from pymetis.classes.products import PipelineProduct
from pymetis.classes.products import DetectorSpecificProduct


class LinGainProduct(DetectorSpecificProduct, ABC):
    """ Common base class for all linearity and gain products. Just sets `group`, `level` and `frame_type`. """
    group = cpl.ui.Frame.FrameGroup.PRODUCT
    level = cpl.ui.Frame.FrameLevel.FINAL
    frame_type = cpl.ui.Frame.FrameType.IMAGE


class MetisDetLinGainImpl(RawImageProcessor, ABC):
    class InputSet(RawImageProcessor.InputSet):
        class RawInput(RawInput):
            _tags: re.Pattern = re.compile(r"DETLIN_(?P<detector>2RG|GEO|IFU)_RAW")
            _description: str = "Raw data for non-linearity determination."

        class WcuOffInput(RawInput):
            _title: str = "WCU off raw"
            _tags: re.Pattern = re.compile(r"(?P<band>LM|N|IFU)_WCU_OFF_RAW")
            _description: str = "Raw data for dark subtraction in other recipes."
            _required: bool = False # FixMe This is just to shut EDPS up

        class BadpixMapInput(OptionalInputMixin, BadpixMapInput):
            _required: bool = False


    class ProductGain(LinGainProduct):
        _description = "Gain map"
        _oca_keywords = {'PRO.CATG'}

        @classmethod
        def tag(cls) -> str:
            return rf"GAIN_MAP_{cls.detector():s}"

    class ProductLinearity(LinGainProduct):
        _description = "Linearity map"
        _oca_keywords = {'PRO.CATG'}

        @classmethod
        def tag(cls) -> str:
            return rf"LINEARITY_{cls.detector():s}"

    class ProductBadpixMap(LinGainProduct):
        _description = "Bad pixel map"
        _oca_keywords = {'PRO.CATG'}

        @classmethod
        def tag(cls) -> str:
            return rf"BADPIX_MAP_{cls.detector():s}"

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

        product_gain_map = self.ProductGain(self, header, gain_image)
        product_linearity = self.ProductLinearity(self, header, linearity_image)
        product_badpix_map = self.ProductBadpixMap(self, header, badpix_map)

        return [product_gain_map, product_linearity, product_badpix_map]

    def _dispatch_child_class(self) -> type["MetisDetLinGainImpl"]:
        return {
            '2RG': Metis2rgLinGainImpl,
            'GEO': MetisGeoLinGainImpl,
            'IFU': MetisIfuLinGainImpl,
        }[self.inputset.detector]


class Metis2rgLinGainImpl(MetisDetLinGainImpl):
    _detector = '2RG'

    class ProductGain(MetisDetLinGainImpl.ProductGain):
        _detector = '2RG'

    class ProductLinearity(MetisDetLinGainImpl.ProductLinearity):
        _detector = '2RG'

    class ProductBadpixMap(MetisDetLinGainImpl.ProductBadpixMap):
        _detector = '2RG'


class MetisGeoLinGainImpl(MetisDetLinGainImpl):
    _detector = 'GEO'

    class ProductGain(MetisDetLinGainImpl.ProductGain):
        _detector = 'GEO'

    class ProductLinearity(MetisDetLinGainImpl.ProductLinearity):
        _detector = 'GEO'

    class ProductBadpixMap(MetisDetLinGainImpl.ProductBadpixMap):
        _detector = 'GEO'


class MetisIfuLinGainImpl(MetisDetLinGainImpl):
    _detector = 'IFU'

    class ProductGain(MetisDetLinGainImpl.ProductGain):
        _detector = 'IFU'

    class ProductLinearity(MetisDetLinGainImpl.ProductLinearity):
        _detector = 'IFU'

    class ProductBadpixMap(MetisDetLinGainImpl.ProductBadpixMap):
        _detector = 'IFU'



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

    _matched_keywords: {str} = {}
    _algorithm = """Subtract instrument dark (hdrl_imagelist_sub_image).
    Compute mean and variance for each frame.
    Gain is determined as the slope of variance against mean (metis_derive_gain).
    Fit polynomial of value as a function of DIT and illumination level for each pixel (metis_derive_nonlinearity).
    Flag pixels with coefficients significantly different from the mean of all pixels (hdrl_bpm_fit_compute)."""

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
