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

from pyesorex.parameter import ParameterList, ParameterValue

from pymetis.classes.dataitems.img.basicreduced import NSciCalibrated, NSciRestored
from pymetis.classes.recipes import MetisRecipe, MetisRecipeImpl
from pymetis.classes.inputs import PipelineInputSet, SinglePipelineInput


class MetisNImgRestoreImpl(MetisRecipeImpl):
    class InputSet(PipelineInputSet):
        class CalibratedInput(SinglePipelineInput):
            Item = NSciCalibrated

    ProductRestored = NSciRestored

    def process_images(self):
        header = self._create_dummy_header()
        image = self._create_dummy_image()
        product = self.ProductRestored(header, image)

        return {product}    # ToDo is just a dummy for now


class MetisNImgRestore(MetisRecipe):
    _name = "metis_n_img_restore"
    _version = "0.1"
    _author = "Martin Baláž, A*"
    _email = "martin.balaz@univie.ac.at"
    _synopsis = "Restore a single positive beam from chop-nod difference image."

    _matched_keywords = {'DRS.FILTER'}
    _algorithm = """Call metis_cutout_region to cut regions around beams
    Add regions with appropriate signs with `hdrl_imagelist_collapse`"""

    parameters = ParameterList([
        ParameterValue(
            name=f"{_name}.cutout_size",
            context=_name,
            description="Name of the method used to combine the input images",
            default=0,
        ),
    ])

    implementation_class = MetisNImgRestoreImpl
