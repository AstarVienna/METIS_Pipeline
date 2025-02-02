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
from typing import Dict

import cpl
from cpl.core import Msg

from pymetis.inputs.common import RawInput, LinearityInput, BadpixMapInput, PersistenceMapInput, GainMapInput
from pymetis.base import MetisRecipe
from pymetis.base.product import PipelineProduct, DetectorSpecificProduct
from pymetis.mixins.detector import Detector2rgMixin, DetectorGeoMixin, DetectorIfuMixin
from pymetis.prefab.rawimage import RawImageProcessor



class MetisDetDarkImpl(RawImageProcessor):
    class InputSet(RawImageProcessor.InputSet):
        class RawInput(RawInput):
            _tags = re.compile(r"DARK_(?P<detector>2RG|GEO|IFU)_RAW")

        def __init__(self, frameset: cpl.ui.FrameSet):
            super().__init__(frameset)
            self.linearity = LinearityInput(frameset, required=False) # But should be
            self.badpix_map = BadpixMapInput(frameset, required=False)
            self.persistence_map = PersistenceMapInput(frameset, required=False) # But should be
            self.gain_map = GainMapInput(frameset, required=False) # But should be

            self.inputs |= {self.linearity, self.badpix_map, self.persistence_map, self.gain_map}

    class Product(DetectorSpecificProduct):
        group = cpl.ui.Frame.FrameGroup.PRODUCT
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE

        @property
        def tag(self) -> str:
            return rf"MASTER_DARK_{self.detector:s}"

        @property
        def output_file_name(self) -> str:
            """ Form the output file name (the detector part is variable here) """
            return rf"{self.category}.fits"

    def process_images(self) -> Dict[str, PipelineProduct]:
        # By default, images are loaded as Python float data. Raw image
        # data which is usually represented as 2-byte integer data in a
        # FITS file is converted on the fly when an image is loaded from
        # a file. It is, however, also possible to load images without
        # performing this conversion.

        # Flat field preparation: subtract bias and normalize it to median 1
        # Msg.info(self.__class__.__qualname__, "Preparing flat field")
        # if flat_image:
        #     if bias_image:
        #         flat_image.subtract(bias_image)
        #     median = flat_image.get_median()
        #     flat_image.divide_scalar(median)

        # Combine the images in the image list using the image stacking
        # option requested by the user.
        method = self.parameters["metis_det_dark.stacking.method"].value
        Msg.info(self.__class__.__qualname__, f"Combining images using method {method!r}")

        # TODO: preprocessing steps like persistence correction / nonlinearity (or not)
        raw_images = self.inputset.load_raw_images()
        combined_image = self.combine_images(raw_images, method)
        header = cpl.core.PropertyList.load(self.inputset.raw.frameset[0].file, 0)

        return {
            fr'METIS_{self.detector}_DARK':
                self.Product(self, header, combined_image, detector=self.detector),
        }


class Metis2rgDarkImpl(Detector2rgMixin, MetisDetDarkImpl):
    class InputSet(MetisDetDarkImpl.InputSet):
        class RawDarkInput(RawInput):
            _tags = re.compile(r"DARK_2RG_RAW")


class MetisGeoDarkImpl(DetectorGeoMixin, MetisDetDarkImpl):
    class InputSet(MetisDetDarkImpl.InputSet):
        class RawDarkInput(RawInput):
            _tags = re.compile(r"DARK_GEO_RAW")


class MetisIfuDarkImpl(DetectorIfuMixin, MetisDetDarkImpl):
    class InputSet(MetisDetDarkImpl.InputSet):
        class RawDarkInput(RawInput):
            _tags = re.compile(r"DARK_IFU_RAW")


class MetisDetDark(MetisRecipe):
    # Fill in recipe information
    _name = "metis_det_dark"
    _version = "0.1"
    _author = "Kieran Chi-Hung Hugo Martin"
    _email = "hugo@buddelmeijer.nl"
    _synopsis = "Create master dark"
    _description = (
        "Prototype to create a METIS masterdark."
    )

    parameters = cpl.ui.ParameterList([
        cpl.ui.ParameterEnum(
            name=f"{_name}.stacking.method",
            context=_name,
            description="Name of the method used to combine the input images",
            default="average",
            alternatives=("add", "average", "median", "sigclip"),
        ),
    ])

    implementation_class = MetisDetDarkImpl

    def dispatch_implementation_class(self, frameset: cpl.ui.FrameSet) -> type[MetisDetDarkImpl]:
        """
        Find the implementation class based on the detector specified in the inputset's tags.
        Tries to instantiate the RawInput and use its detector attribute to determine the correct implementation class.

        Parameters:
        frameset: cpl.ui.FrameSet
            The input data used to create the `RawInput` object for dispatching the
            implementation class.

        Returns:
        Type[Metis2rgDarkImpl | MetisGeoDarkImpl | MetisIfuDarkImpl]
            The implementation class corresponding to the detector specified in the
            `RawInput` object.

        Raises:
        KeyError
            If the detector obtained from the `RawInput` object is not found in the
            implementation mapping.
        """
        inputset = self.implementation_class.InputSet.RawInput(frameset)
        return {
            '2RG': Metis2rgDarkImpl,
            'GEO': MetisGeoDarkImpl,
            'IFU': MetisIfuDarkImpl,
        }[inputset.detector]
