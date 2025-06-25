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

from pymetis.classes.dataitems.dataitem import DataItem
from pymetis.classes.dataitems.distortion.table import IfuDistortionTable
from pymetis.classes.dataitems.ifu.raw import IfuSciRaw, IfuSkyRaw, IfuRaw
from pymetis.classes.dataitems.ifu.ifu import IfuCombined, IfuReduced, IfuStdReducedCube, \
    IfuSciReduced, IfuStdReduced, IfuReducedCube, IfuStdCombined, IfuSciCombined, \
    IfuSciReducedCube
from pymetis.classes.dataitems.ifu.background import IfuBackground, IfuStdBackground, IfuSciBackground
from pymetis.classes.dataitems.rsrf import RsrfIfu
from pymetis.classes.mixins import TargetStdMixin, TargetSciMixin
from pymetis.classes.recipes import MetisRecipe
from pymetis.classes.prefab.darkimage import DarkImageProcessor
from pymetis.classes.inputs import (SinglePipelineInput, RawInput, MasterDarkInput, WavecalInput,
                                    PersistenceInputSetMixin, GainMapInputSetMixin, LinearityInputSetMixin)
from pymetis.classes.products import PipelineProduct, TargetSpecificProduct, PipelineImageProduct


class MetisIfuReduceImpl(DarkImageProcessor):
    target: Literal["SCI"] | Literal["STD"] = None

    class InputSet(GainMapInputSetMixin, PersistenceInputSetMixin, LinearityInputSetMixin, DarkImageProcessor.InputSet):
        class RawInput(RawInput):
            Item = IfuRaw
            _tags: re.Pattern = re.compile(r"IFU_(?P<target>SCI|STD)_RAW")

        class RawSkyInput(RawInput):
            Item = IfuSkyRaw
            _tags: re.Pattern = re.compile(r"IFU_SKY_RAW")

        class MasterDarkInput(MasterDarkInput):
            _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.RAW

        WavecalInput = WavecalInput

        class DistortionTableInput(SinglePipelineInput):
            _tags: re.Pattern = re.compile(r"IFU_DISTORTION_TABLE")
            Item: type[DataItem] = IfuDistortionTable

        class RsrfInput(SinglePipelineInput):
            _tags: re.Pattern = re.compile(r"RSRF_IFU")
            Item: type[DataItem] = RsrfIfu

    class ProductReduced(TargetSpecificProduct, PipelineImageProduct):
        Item = IfuReduced

    class ProductBackground(TargetSpecificProduct, PipelineImageProduct):
        Item = IfuBackground

    class ProductReducedCube(TargetSpecificProduct, PipelineImageProduct):
        Item = IfuReducedCube

    class ProductCombined(TargetSpecificProduct, PipelineImageProduct):
        Item = IfuCombined

    def process_images(self) -> set[PipelineProduct]:
        # do something... a lot of something

        header = cpl.core.PropertyList()
        images = self.inputset.load_raw_images()
        image = self.combine_images(images, "add")

        return {
            self.ProductReduced(self, header, image),
            self.ProductBackground(self, header, image),
            self.ProductReducedCube(self, header, image),
            self.ProductCombined(self, header, image),
        }

    def _dispatch_child_class(self):
        return {
            'STD': MetisIfuReduceStdImpl,
            'SCI': MetisIfuReduceSciImpl,
        }[self.inputset.target]


class MetisIfuReduceStdImpl(MetisIfuReduceImpl):
    class ProductReduced(TargetStdMixin, MetisIfuReduceImpl.ProductReduced):
        Item = IfuStdReduced

    class ProductBackground(TargetStdMixin, MetisIfuReduceImpl.ProductBackground):
        Item = IfuStdBackground

    class ProductCombined(TargetStdMixin, MetisIfuReduceImpl.ProductCombined):
        Item = IfuStdCombined

    class ProductReducedCube(TargetStdMixin, MetisIfuReduceImpl.ProductReducedCube):
        Item = IfuStdReducedCube


class MetisIfuReduceSciImpl(MetisIfuReduceImpl):
    class ProductReduced(TargetSciMixin, MetisIfuReduceImpl.ProductReduced):
        Item = IfuSciReduced

    class ProductBackground(TargetSciMixin, MetisIfuReduceImpl.ProductBackground):
        Item = IfuSciBackground

    class ProductCombined(TargetSciMixin, MetisIfuReduceImpl.ProductCombined):
        Item = IfuSciCombined

    class ProductReducedCube(TargetSciMixin, MetisIfuReduceImpl.ProductReducedCube):
        Item = IfuSciReducedCube


class MetisIfuReduce(MetisRecipe):
    _name: str = "metis_ifu_reduce"
    _version: str = "0.1"
    _author: str = "Martin Baláž, A*"
    _email: str = "martin.balaz@univie.ac.at"
    _synopsis: str = "Reduce raw science exposures of the IFU."
    _description: str = (
        "Currently just a skeleton prototype."
    )

    _matched_keywords: set[str] = {'DET.DIT', 'DET.NDIT', 'DRS.IFU'}
    _algorithm = """Subtract dark, divide by master flat
    Analyse and optionally remove masked regions and correct crosstalk and ghosts
    Estimate stray light and subtract
    Estimate background from dithered science exposures or blank-sky exposures and subtract
    Rectify spectra and assemble cube
    Extract 1D object spectrum"""

    implementation_class = MetisIfuReduceImpl
