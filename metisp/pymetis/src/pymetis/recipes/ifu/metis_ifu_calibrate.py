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

from pymetis.base import MetisRecipe, MetisRecipeImpl
from pymetis.base.product import PipelineProduct
from pymetis.inputs import SinglePipelineInput, PipelineInputSet


class MetisIfuCalibrateImpl(MetisRecipeImpl):
    class InputSet(PipelineInputSet):
        class SciReducedInput(SinglePipelineInput):
            _title: str = "science reduced"
            _group = cpl.ui.Frame.FrameGroup.CALIB
            _tags: re.Pattern = re.compile(r"IFU_SCI_REDUCED")
            _description = "Reduced 2D detector image of science object."

        class TelluricInput(SinglePipelineInput):
            _title: str = "telluric correction"
            _group = cpl.ui.Frame.FrameGroup.CALIB
            _tags: re.Pattern = re.compile(r"IFU_TELLURIC")
            _description = "Telluric absorption correction."

        class FluxcalTabInput(SinglePipelineInput):
            _title: str = "flux calibration table"
            _group = cpl.ui.Frame.FrameGroup.CALIB
            _tags: re.Pattern = re.compile(r"FLUXCAL_TAB")
            _description = "Conversion between instrumental and physical flux units."

    class ProductSciCubeCalibrated(PipelineProduct):
        _tag = rf"IFU_SCI_CUBE_CALIBRATED"
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE
        description = "A telluric absorption corrected rectified spectral cube with a linear wavelength grid."
        oca_keywords = ['PRO.CATG', 'DRS.IFU']

    def process_images(self) -> [PipelineProduct]:
        # self.correct_telluric()
        # self.apply_fluxcal()

        header = self._create_dummy_header()
        image = self._create_dummy_image()

        product_scc = self.ProductSciCubeCalibrated(self, header, image)

        return [product_scc]


class MetisIfuCalibrate(MetisRecipe):
    _name: str = "metis_ifu_calibrate"
    _version: str = "0.1"
    _author: str = "Martin Baláž, A*"
    _email: str = "martin.balaz@univie.ac.at"
    _synopsis: str = "Calibrate IFU science data"
    _description: str = (
        "Currently just a skeleton prototype."
    )

    implementation_class =  MetisIfuCalibrateImpl
