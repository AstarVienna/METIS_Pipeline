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

import cpl
from cpl.core import Msg

from pymetis.base.recipe import MetisRecipe
from pymetis.base.product import PipelineProduct
from pymetis.inputs import RawInput, SinglePipelineInput
from pymetis.inputs.mixins import PersistenceInputSetMixin, LinearityInputSetMixin, GainMapInputSetMixin
from pymetis.prefab.rawimage import RawImageProcessor


class MetisLmImgDistortionImpl(RawImageProcessor):
    class InputSet(PersistenceInputSetMixin, LinearityInputSetMixin, GainMapInputSetMixin, RawImageProcessor.InputSet):
        class RawInput(RawInput):
            _tags: re.Pattern = re.compile(r"LM_WCU_OFF_RAW")
            _description = "Raw data for dark subtraction in other recipes."

        class DistortionInput(SinglePipelineInput):
            _tags: re.Pattern = re.compile(r"LM_DISTORTION_RAW")
            _title: str = "Distortion map"
            _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB
            _description = "Images of grid mask in WCU-FP2 or CFO-FP2."

        class PinholeTableInput(SinglePipelineInput):
            _tags: re.Pattern = re.compile(r"PINHOLE_TABLE")
            _title: str = "pinhole table"
            _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.CALIB
            _description = "Table of pinhole locations"


    class ProductLmDistortionTable(PipelineProduct):
        _tag = r"LM_DISTORTION_TABLE"
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.TABLE
        description = "Table of distortion information"
        oca_keywords = ['PRO.CATG', 'DRS.FILTER']

    class ProductLmDistortionMap(PipelineProduct):
        _tag = r"LM_DISTORTION_MAP"
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE
        description = "Map of pixel scale across the detector"
        oca_keywords = ['PRO.CATG', 'DRS.FILTER']

    class ProductLmDistortionReduced(PipelineProduct):
        _tag = r"LM_DIST_REDUCED"
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE
        description = "Table of polynomial coefficients for distortion correction"
        oca_keywords = ['PRO.CATG', 'DRS.FILTER']

    def process_images(self) -> [PipelineProduct]:
        raw_images = cpl.core.ImageList()

        for idx, frame in enumerate(self.inputset.raw.frameset):
            Msg.info(self.name, f"Loading raw image {frame.file}")

            if idx == 0:
                self.header = cpl.core.PropertyList.load(frame.file, 0)

            raw_image = cpl.core.Image.load(frame.file, extension=1)
            raw_images.append(raw_image)

        combined_image = self.combine_images(raw_images, "average")

        return [
            self.ProductLmDistortionTable(self, self.header, combined_image),
            self.ProductLmDistortionMap(self, self.header, combined_image),
            self.ProductLmDistortionReduced(self, self.header, combined_image),
        ]


class MetisLmImgDistortion(MetisRecipe):
    _name: str = "metis_lm_img_distortion"
    _version: str = "0.1"
    _author: str = "Chi-Hung Yan, A*"
    _email: str = "chyan@asiaa.sinica.edu.tw"
    _synopsis: str = "Determine optical distortion coefficients for the LM imager."
    _description: str = (
        "Currently just a skeleton prototype."
    )

    _matched_keywords: [str] = ['DRS.FILTER']
    _algorithm = """Subtract background image with `hdrl_imagelist_sub_image`.
    Measure location of point source images in frames with `hdrl_catalogue_create`.
    Call metis_fit_distortion to fit polynomial coefficients to deviations from grid positions."""

    parameters = cpl.ui.ParameterList([
        cpl.ui.ParameterEnum(
            name="metis_lm_img_distortion.stacking.method",
            context="metis_lm_img_distortion",
            description="Name of the method used to combine the input images",
            default="average",
            alternatives=("add", "average", "median", "sigclip"),
        ),
    ])

    implementation_class = MetisLmImgDistortionImpl
