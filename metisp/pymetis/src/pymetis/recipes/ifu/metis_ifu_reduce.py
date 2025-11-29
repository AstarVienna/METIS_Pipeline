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

from typing import Literal

import cpl
from pyesorex.parameter import ParameterList, ParameterEnum, ParameterValue

from pymetis.classes.dataitems import DataItem, Hdu
from pymetis.classes.mixins import BandIfuMixin, DetectorIfuMixin
from pymetis.dataitems.distortion.table import IfuDistortionTable
from pymetis.dataitems.ifu.raw import IfuSkyRaw, IfuRaw
from pymetis.dataitems.ifu.ifu import IfuCombined, IfuReduced, IfuReducedCube
from pymetis.dataitems.ifu.background import IfuBackground
from pymetis.dataitems.rsrf import RsrfIfu
from pymetis.classes.recipes import MetisRecipe
from pymetis.classes.prefab.darkimage import DarkImageProcessor
from pymetis.classes.inputs import (SinglePipelineInput, RawInput, WavecalInput,
                                    OptionalInputMixin, PersistenceMapInput, GainMapInput, LinearityInput)

from pymetis.utils.dummy import create_dummy_header


class MetisIfuReduceImpl(BandIfuMixin, DetectorIfuMixin, DarkImageProcessor):
    class InputSet(DarkImageProcessor.InputSet):
        class RawInput(RawInput):
            Item = IfuRaw

        class RawSkyInput(RawInput):
            Item = IfuSkyRaw

        class PersistenceMapInput(OptionalInputMixin, PersistenceMapInput):
            pass

        class GainMapInput(GainMapInput):
            pass

        class LinearityInput(LinearityInput):
            pass

        class WavecalInput(WavecalInput):
            pass # We need to create a new class here, not reuse the old one!

        class DistortionTableInput(SinglePipelineInput):
            Item = IfuDistortionTable

        class RsrfInput(SinglePipelineInput):
            Item = RsrfIfu

    ProductReduced = IfuReduced
    ProductBackground = IfuBackground
    ProductReducedCube = IfuReducedCube
    ProductCombined = IfuCombined

    def _process_single_detector(self, detector: Literal[1, 2, 3, 4]) -> dict[str, Hdu]:
        """
        Process exposures for a single detector of the IFU.

        Parameters
        ----------
        detector : Literal[1, 2, 3, 4]

        Returns
        -------
        dict[str, Hdu]
            Processed and background images for the given detector.
        """

        det = rf'{detector:1d}'
        raw_images = self.inputset.raw.use().load_data(extension=rf'DET{det}.DATA')

        #TBD: implement actual reduction steps
        combined_image = self.combine_images(raw_images, "average")

        header_image = create_dummy_header()
        header_image.append(cpl.core.Property("EXTNAME", cpl.core.Type.STRING, rf'DET{det}.DATA'))

        header_background = create_dummy_header()
        header_background.append(cpl.core.Property("EXTNAME", cpl.core.Type.STRING, rf'DET{det}.DATA'))

        return {
            'IMAGE': Hdu(header_image, combined_image, name=rf'DET{det}.DATA'),
            'BACKGROUND': Hdu(header_background, combined_image, name=rf'DET{det}.DATA'),
        }

    def process(self) -> set[DataItem]:
        header_reduced = create_dummy_header()
        header_background = create_dummy_header()
        header_reduced_cube = create_dummy_header()
        header_combined_cube = create_dummy_header()

        output = [self._process_single_detector(det) for det in [1, 2, 3, 4]]
        primary_header = cpl.core.PropertyList()
        raw_images = self.inputset.raw.load_data('DET1.DATA')
        image = self.combine_images(raw_images, "add")

        product_reduced = self.ProductReduced(
            header_reduced,
            *[out['IMAGE'] for out in output],
        )

        product_background = self.ProductBackground(
            header_background,
            *[out['BACKGROUND'] for out in output],
        )

        # cready dummy image for cube outputs
        raw_images = self.inputset.raw.use().load_data(extension=rf'DET1.DATA')
        combined_image = self.combine_images(raw_images, "average")

        return {
            product_reduced,
            product_background,
            self.ProductReducedCube(
                primary_header,
                Hdu(header_reduced_cube, combined_image, name='IMAGE'),
            ),
            self.ProductCombined(
                primary_header,
                Hdu(header_combined_cube, image, name='DET1.DATA'),
            ),
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

    # Define the parameters as required by the recipe. Again, this is needed by `pyesorex`.
    parameters = ParameterList([
        ParameterEnum(
            name=f"{_name}.stacking.method",
            context=_name,
            description="Name of the method used to combine the input images",
            default="average",
            alternatives=("add", "average", "median", "sigclip"),
        ),
    ])

    Impl = MetisIfuReduceImpl
