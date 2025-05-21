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

from pymetis.classes.recipes import MetisRecipeImpl
from pymetis.classes.recipes import MetisRecipe
from pymetis.classes.inputs import PipelineInputSet, SinglePipelineInput
from pymetis.classes.products import PipelineProduct, PipelineImageProduct


class MetisNImgRestoreImpl(MetisRecipeImpl):
    class InputSet(PipelineInputSet):
        class CalibratedInput(SinglePipelineInput):
            _tags: re.Pattern = re.compile(r'N_SCI_CALIBRATED')
            _description: str = "N band image with flux calibration and distortion information"
            _group = cpl.ui.Frame.FrameGroup.CALIB
            _title = "N science calibrated"

    class ProductRestored(PipelineImageProduct):
        _tag: re.Pattern = r'N_SCI_RESTORED'
        _description: str = "N band image with a single positive beam restored from chop-nod image"
        level = cpl.ui.Frame.FrameLevel.FINAL
        _oca_keywords = {'PRO.CATG', 'DRS.FILTER'}

    def process_images(self) -> list[PipelineProduct]:
        header = self._create_dummy_header()
        image = self._create_dummy_image()

        product = self.ProductRestored(self, header, image)

        return [product]    # ToDo is just a dummy for now


class MetisNImgRestore(MetisRecipe):
    _name: str = "metis_n_img_restore"
    _version: str = "0.1"
    _author: str = "Martin Baláž, A*"
    _email: str = "martin.balaz@univie.ac.at"
    _synopsis: str = "Restore a single positive beam from chop-nod difference image."

    _matched_keywords: set[str] = {'DRS.FILTER'}
    _algorithm: str = """Call metis_cutout_region to cut regions around beams
    Add regions with appropriate signs with `hdrl_imagelist_collapse`"""

    parameters = cpl.ui.ParameterList([
        cpl.ui.ParameterValue(
            name=f"{_name}.cutout_size",
            context=_name,
            description="Name of the method used to combine the input images",
            default=0,
        ),
    ])

    implementation_class = MetisNImgRestoreImpl
