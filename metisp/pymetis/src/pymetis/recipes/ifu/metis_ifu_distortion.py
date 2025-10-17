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
from cpl.core import Msg

from pymetis.classes.dataitems.dataitem import DataItem
from pymetis.classes.dataitems.hdu import Hdu
from pymetis.dataitems.distortion import IfuDistortionRaw, IfuDistortionTable, IfuDistortionReduced
from pymetis.classes.recipes import MetisRecipe
from pymetis.classes.inputs import RawInput, MasterDarkInput
from pymetis.classes.inputs import PinholeTableInput
from pymetis.classes.inputs import PersistenceInputSetMixin, LinearityInputSetMixin, GainMapInputSetMixin
from pymetis.classes.prefab.darkimage import DarkImageProcessor
from pymetis.utils.dummy import create_dummy_table, create_dummy_header


class MetisIfuDistortionImpl(DarkImageProcessor):
    class InputSet(LinearityInputSetMixin, GainMapInputSetMixin, PersistenceInputSetMixin, DarkImageProcessor.InputSet):
        MasterDarkInput = MasterDarkInput
        PinholeTableInput = PinholeTableInput

        class RawInput(RawInput):
            Item = IfuDistortionRaw

    ProductDistortionTable = IfuDistortionTable
    ProductDistortionReduced = IfuDistortionReduced

    def _process_single_detector(self, detector: Literal[1, 2, 3, 4]) -> dict[str, Hdu]:
        """
        Find the distortion coeeficients for a single detector of the IFU.

        Parameters
        ----------
        detector : Literal[1, 2, 3, 4] # FixMe: Maybe make this fully customizable for any detector count?

        Returns
        -------
        dict[str, Hdu]
            Distortion coefficients for a single detector of the IFU, in a form of table and image
            # FixMe this does not make much sense but works for now [MB]
        """

        det = rf'{detector:1d}'
        raw_images = self.inputset.raw.use().load_data(extension=rf'DET{detector:1d}.DATA')
        combined_image = self.combine_images(raw_images, "average")

        header_table = create_dummy_header()
        header_table.append(cpl.core.Property("EXTNAME", cpl.core.Type.STRING, rf'DET{detector:1d}'))
        table = create_dummy_table(14)

        header_image = create_dummy_header()
        header_image.append(cpl.core.Property("EXTNAME", cpl.core.Type.STRING, rf'DET{detector:1d}'))

        return {
            'TABLE': Hdu(header_table, table, name=rf'DET{det}'),
            'IMAGE': Hdu(header_image, combined_image, name=rf'DET{det}'),
        }

    def process(self) -> set[DataItem]:
        header = create_dummy_header()

        output = [self._process_single_detector(det) for det in [1, 2, 3, 4]]

        product_distortion = self.ProductDistortionTable(
            header,
            *[out['TABLE'] for out in output],
        )
        product_distortion_reduced = self.ProductDistortionReduced(
            header,
            *[out['IMAGE'] for out in output],
        )

        return {product_distortion, product_distortion_reduced}


class MetisIfuDistortion(MetisRecipe):
    _name = "metis_ifu_distortion"
    _version = "0.1"
    _author = "Martin Baláž, A*"
    _email = "martin.balaz@univie.ac.at"
    _synopsis = "Reduce raw science exposures of the IFU."
    _description = (
        "Currently just a skeleton prototype."
    )

    _matched_keywords = {'DRS.IFU'}
    _algorithm = """Calculate table mapping pixel position to position on sky."""

    Impl = MetisIfuDistortionImpl
