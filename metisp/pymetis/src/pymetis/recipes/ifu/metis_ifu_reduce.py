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
from typing import Literal

from pymetis.base import MetisRecipe
from pymetis.products import TargetSpecificProduct
from pymetis.products.product import PipelineProduct
from pymetis.inputs import SinglePipelineInput, PipelineInputSet
from pymetis.inputs.common import RawInput, MasterDarkInput
from pymetis.inputs.mixins import PersistenceInputSetMixin, GainMapInputSetMixin, LinearityInputSetMixin

from pymetis.prefab.darkimage import DarkImageProcessor


class MetisIfuReduceImpl(DarkImageProcessor):
    target: Literal["SCI"] | Literal["STD"] = None

    class InputSet(GainMapInputSetMixin, PersistenceInputSetMixin, LinearityInputSetMixin, DarkImageProcessor.InputSet):
        detector = "IFU"

        class RawInput(RawInput):
            _tags: re.Pattern = re.compile(r"IFU_(?P<target>SCI|STD)_RAW")
            _description: str = "IFU raw exposure of a science object"

        class RawSkyInput(RawInput):
            _tags: re.Pattern = re.compile(r"IFU_SKY_RAW")
            _title: str = "blank sky image"
            _description: str = "Blank sky image"

        class MasterDarkInput(MasterDarkInput):
            _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.RAW

        class WavecalInput(SinglePipelineInput):
            _tags: re.Pattern = re.compile(r"IFU_WAVECAL")
            _group = cpl.ui.Frame.FrameGroup.CALIB
            _title: str = "Wavelength calibration"
            _description = "Image with wavelength at each pixel"

        class DistortionTableInput(SinglePipelineInput):
            _tags: re.Pattern = re.compile(r"IFU_DISTORTION_TABLE")
            _group = cpl.ui.Frame.FrameGroup.CALIB
            _title: str = "Distortion table"
            _description = "Table of distortion coefficients for an IFU data set"

        class RsrfInput(SinglePipelineInput):
            _tags: re.Pattern = re.compile(r"RSRF_IFU")
            _group = cpl.ui.Frame.FrameGroup.CALIB
            _title: str = "RSRF"
            _description = "2D relative spectral response function"

        MasterDarkInput = MasterDarkInput


    class ProductReduced(TargetSpecificProduct):
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE
        _description = "Table of polynomial coefficients for distortion correction"
        _oca_keywords: {str} = {'PRO.CATG', 'DRS.IFU'}

        @classmethod
        def tag(cls) -> str:
            return rf"IFU_{cls.target():s}_REDUCED"

    class ProductBackground(TargetSpecificProduct):
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE
        _description = "Reduced 2D detector image of background."
        _oca_keywords: {str} = {'PRO.CATG', 'DRS.IFU'}

        @classmethod
        def tag(cls) -> str:
            return rf"IFU_{cls.target():s}_BACKGROUND"

    class ProductReducedCube(TargetSpecificProduct):
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE
        _description = "Reduced 2D detector image of spectroscopic flux standard star."
        _oca_keywords: {str} = {'PRO.CATG', 'DRS.IFU'}

        @classmethod
        def tag(cls) -> str:
            return rf"IFU_{cls.target():s}_REDUCED_CUBE"

    class ProductCombined(TargetSpecificProduct):
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE
        _description = "Spectral cube of standard star, combining multiple exposures."
        _oca_keywords: {str} = {'PRO.CATG', 'DRS.IFU'}

        @classmethod
        def tag(cls) -> str:
            return rf"IFU_{cls.target():s}_COMBINED"

    def process_images(self) -> [PipelineProduct]:
        # do something... a lot of something

        self.target = self.inputset.tag_parameters["target"]

        header = cpl.core.PropertyList()
        images = self.inputset.load_raw_images()
        image = self.combine_images(images, "add")

        return [
            self.ProductReduced(self, header, image),
            self.ProductBackground(self, header, image),
            self.ProductReducedCube(self, header, image),
            self.ProductCombined(self, header, image),
        ]


class MetisIfuReduceStdImpl(MetisIfuReduceImpl):
    class ProductReduced(MetisIfuReduceImpl.ProductReduced):
        _target = 'STD'

    class ProductBackground(MetisIfuReduceImpl.ProductBackground):
        _target = 'STD'

    class ProductCombined(MetisIfuReduceImpl.ProductCombined):
        _target = 'STD'

    class ProductReducedCube(MetisIfuReduceImpl.ProductReducedCube):
        _target = 'STD'


class MetisIfuReduceSciImpl(MetisIfuReduceImpl):
    class ProductReduced(MetisIfuReduceImpl.ProductReduced):
        _target = 'SCI'

    class ProductBackground(MetisIfuReduceImpl.ProductBackground):
        _target = 'SCI'

    class ProductCombined(MetisIfuReduceImpl.ProductCombined):
        _target = 'SCI'

    class ProductReducedCube(MetisIfuReduceImpl.ProductReducedCube):
        _target = 'SCI'


class MetisIfuReduce(MetisRecipe):
    _name: str = "metis_ifu_reduce"
    _version: str = "0.1"
    _author: str = "Martin Baláž, A*"
    _email: str = "martin.balaz@univie.ac.at"
    _synopsis: str = "Reduce raw science exposures of the IFU."
    _description: str = (
        "Currently just a skeleton prototype."
    )

    _matched_keywords: {str} = {'DET.DIT', 'DET.NDIT', 'DRS.IFU'}
    _algorithm = """Subtract dark, divide by master flat
    Analyse and optionally remove masked regions and correct crosstalk and ghosts
    Estimate stray light and subtract
    Estimate background from dithered science exposures or blank-sky exposures and subtract
    Rectify spectra and assemble cube
    Extract 1D object spectrum"""

    implementation_class = MetisIfuReduceImpl

    def dispatch_implementation_class(self, inputset: PipelineInputSet) -> type["MetisRecipeImpl"]:
        return {
            'STD': MetisIfuReduceStdImpl,
            'SCI': MetisIfuReduceSciImpl,
        }[inputset.target]