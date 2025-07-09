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
from pymetis.classes.dataitems.wavecal import IfuWavecalRaw, IfuWavecal
from pymetis.classes.mixins import BandIfuMixin, DetectorIfuMixin
from pymetis.classes.recipes import MetisRecipe
from pymetis.classes.inputs import MasterDarkInput, RawInput, DistortionTableInput
from pymetis.classes.inputs import PersistenceInputSetMixin, LinearityInputSetMixin, GainMapInputSetMixin
from pymetis.classes.prefab.darkimage import DarkImageProcessor


class MetisIfuWavecalImpl(DarkImageProcessor):
    class InputSet(BandIfuMixin, DetectorIfuMixin,
                   PersistenceInputSetMixin, LinearityInputSetMixin, GainMapInputSetMixin,
                   DarkImageProcessor.InputSet):
        class RawInput(RawInput):
            Item = IfuWavecalRaw

        MasterDarkInput = MasterDarkInput
        DistortionTableInput = DistortionTableInput

    ProductIfuWavecal = IfuWavecal

    def process(self) -> set[DataItem]:
        # self.correct_telluric()
        # self.apply_fluxcal()

        header = cpl.core.PropertyList()
        images = self.inputset.load_raw_images()
        image = self.combine_images(images, "add")

        product_wavecal = self.ProductIfuWavecal(header, image)

        return {product_wavecal}


class MetisIfuWavecal(MetisRecipe):
    _name: str = "metis_ifu_wavecal"
    _version: str = "0.1"
    _author: str = "Martin Baláž, A*"
    _email: str = "martin.balaz@univie.ac.at"
    _synopsis: str = "Determine the relative spectral response function"
    _description: str = (
        "Currently just a skeleton prototype."
    )

    _algorithm = """Measure line locations (left and right edges, centroid by Gaussian fit).
        Compute deviation from optical models.
        Compute wavelength solution ξ(x, y, i), λ(x, y, i).
        Compute wavelength map."""
    _matched_keywords: set[str] = {'DET.DIT', 'DET.NDIT', 'DRS.IFU'}

    Impl = MetisIfuWavecalImpl
