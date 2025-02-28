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
from cpl.core import Msg

from pymetis.classes.mixins.detector import Detector2rgMixin, DetectorGeoMixin, DetectorIfuMixin
from pymetis.classes.recipes import MetisRecipe
from pymetis.classes.prefab import RawImageProcessor
from pymetis.classes.inputs import (RawInput, BadpixMapInput, PersistenceMapInput,
                                    LinearityInput, GainMapInput, OptionalInputMixin)
from pymetis.classes.inputs import PersistenceInputSetMixin
from pymetis.classes.products import PipelineProduct
from pymetis.classes.products import DetectorSpecificProduct


class MetisDetDarkImpl(RawImageProcessor, ABC):
    class InputSet(PersistenceInputSetMixin, RawImageProcessor.InputSet):
        class RawInput(RawInput):
            _tags: re.Pattern = re.compile(r"DARK_(?P<detector>2RG|GEO|IFU)_RAW")
            _description: str = "Raw data for creating a master dark."

        class PersistenceMapInput(OptionalInputMixin, PersistenceMapInput): pass
        class BadpixMapInput(OptionalInputMixin, BadpixMapInput): pass
        # FixMe: these two should not be optional, but the current EDPS workflow does not supply them
        class LinearityInput(OptionalInputMixin, LinearityInput): pass
        class GainMapInput(OptionalInputMixin, GainMapInput): pass

    class ProductMasterDark(DetectorSpecificProduct):
        group = cpl.ui.Frame.FrameGroup.PRODUCT
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE
        _oca_keywords = {'PRO.CATG', 'DRS.FILTER'}

        @classmethod
        def description(cls) -> str:
            return f"Master dark frame for '{cls.detector()}' detector data"

        @classmethod
        def tag(cls) -> str:
            return rf"MASTER_DARK_{cls.detector()}"

        @property
        def output_file_name(self) -> str:
            """ Form the output file name (the detector part is variable here) """
            return rf"{self.category}.fits"

    def process_images(self) -> [PipelineProduct]:
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

        product = self.ProductMasterDark(self, header, combined_image)

        return [product]

    def _dispatch_child_class(self) -> type["MetisDetDarkImpl"]:
        """
        Find the implementation class based on the detector specified in the inputset tags.

        Raises:
        KeyError
            If the detector obtained from the inputset is not found in the mapping.
        """
        return {
            '2RG': Metis2rgDarkImpl,
            'GEO': MetisGeoDarkImpl,
            'IFU': MetisIfuDarkImpl,
        }[self.inputset.detector]


class Metis2rgDarkImpl(MetisDetDarkImpl):
    class InputSet(MetisDetDarkImpl.InputSet):
        class RawInput(Detector2rgMixin, MetisDetDarkImpl.InputSet.RawInput):
            pass

    class GainMapInput(Detector2rgMixin, MetisDetDarkImpl.InputSet.GainMapInput):
        pass

    class ProductMasterDark(Detector2rgMixin, MetisDetDarkImpl.ProductMasterDark):
        pass


class MetisGeoDarkImpl(MetisDetDarkImpl):
    class InputSet(MetisDetDarkImpl.InputSet):
        class RawInput(DetectorGeoMixin, MetisDetDarkImpl.InputSet.RawInput):
            pass

    class GainMapInput(DetectorGeoMixin, MetisDetDarkImpl.InputSet.GainMapInput):
        pass

    class ProductMasterDark(DetectorGeoMixin, MetisDetDarkImpl.ProductMasterDark):
        pass


class MetisIfuDarkImpl(MetisDetDarkImpl):
    class InputSet(MetisDetDarkImpl.InputSet):
        class RawInput(DetectorIfuMixin, MetisDetDarkImpl.InputSet.RawInput):
            pass

        class GainMapInput(DetectorIfuMixin, MetisDetDarkImpl.InputSet.GainMapInput):
            pass

    class ProductMasterDark(DetectorIfuMixin, MetisDetDarkImpl.ProductMasterDark):
        pass



class MetisDetDark(MetisRecipe):
    # Fill in recipe information
    _name: str = "metis_det_dark"
    _version: str = "0.1"
    _author: str = "Hugo Buddelmeijer, A*"
    _email: str = "hugo@buddelmeijer.nl"
    _synopsis: str = "Create master dark"
    _description: str = (
        "Prototype to create a METIS masterdark."
    )

    _matched_keywords: {str} = {}
    _algorithm = """Group files by detector and DIT, based on header keywords
    Call function metis_determine_dark for each set of files
    Call metis_update_dark_mask to flag deviant pixels
    """

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
