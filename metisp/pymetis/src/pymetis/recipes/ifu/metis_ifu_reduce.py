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

import cpl

from pymetis.classes.dataitems import DataItem
from pymetis.dataitems.distortion.table import IfuDistortionTable
from pymetis.dataitems.ifu.raw import IfuSkyRaw, IfuRaw
from pymetis.dataitems.ifu.ifu import IfuCombined, IfuReduced, IfuReducedCube
from pymetis.dataitems.ifu.background import IfuBackground
from pymetis.dataitems.rsrf import RsrfIfu
from pymetis.classes.recipes import MetisRecipe
from pymetis.classes.prefab.darkimage import DarkImageProcessor
from pymetis.classes.inputs import (SinglePipelineInput, RawInput, MasterDarkInput, WavecalInput,
                                    PersistenceInputSetMixin, GainMapInputSetMixin, LinearityInputSetMixin)


class MetisIfuReduceImpl(DarkImageProcessor):
    class InputSet(GainMapInputSetMixin, PersistenceInputSetMixin, LinearityInputSetMixin, DarkImageProcessor.InputSet):
        class RawInput(RawInput):
            Item = IfuRaw

        class RawSkyInput(RawInput):
            Item = IfuSkyRaw

        MasterDarkInput = MasterDarkInput
        WavecalInput = WavecalInput

        class DistortionTableInput(SinglePipelineInput):
            Item = IfuDistortionTable

        class RsrfInput(SinglePipelineInput):
            Item = RsrfIfu

    ProductReduced = IfuReduced
    ProductBackground = IfuBackground
    ProductReducedCube = IfuReducedCube
    ProductCombined = IfuCombined

    def process(self) -> set[DataItem]:
        # do something... a lot of something

        header = cpl.core.PropertyList()
        images = self.inputset.raw.load(extension=1)
        image = self.combine_images(images, "add")

        return {
            self.ProductReduced(header, image),
            self.ProductBackground(header, image),
            self.ProductReducedCube(header, image),
            self.ProductCombined(header, image),
        }


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

    Impl = MetisIfuReduceImpl
