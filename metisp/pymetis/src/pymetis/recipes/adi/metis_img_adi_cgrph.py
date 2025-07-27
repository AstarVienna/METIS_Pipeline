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
from pymetis.dataitems import DistortionTable

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

from pyesorex.parameter import ParameterList, ParameterEnum, ParameterRange

from pymetis.classes.dataitems import DataItem
from pymetis.dataitems.background import Background, BackgroundSubtracted
from pymetis.dataitems.img.basicreduced import LmSkyBasicReduced, Calibrated
from pymetis.dataitems.objectcatalog import ObjectCatalog
from pymetis.classes.recipes import MetisRecipe, MetisRecipeImpl
from pymetis.classes.inputs import PipelineInputSet, SinglePipelineInput


class MetisImgAdiCgrphImpl(MetisRecipeImpl):
    class InputSet(PipelineInputSet):
        SciCalibratedInput = SinglePipelineInput[Calibrated]
        DistortionTableInput = SinglePipelineInput[DistortionTable]
        SciThroughputInput = SinglePipelineInput[SciThroughput]
        OffAxisPsfRawInput = SinglePipelineInput[OffAxisPsfRaw]
        OnAxisPsfTemplateInput = SinglePipelineInput[OnAxisPsfTemplate]

    ProductSciCalibrated = CgrphSciCalibrated
    ProductSciCentred = CgrphSciCentred
    ProductCentroidTab = CgrphSciCentred
    ProductSciSpeckle = CgrphSciSpeckle
    ProductSciHiFilt = CgrphSciHiFilt
    ProductSciDerotatedPsfSub = CgrphSciDerotatedPsfSub
    ProductSciDerotated = CgrphSciDerotated
    ProductSciContrastRadProf = CgrphSciContrastRadProf
    ProductSciContrastAdi = CgrphSciContrastAdi
    ProductSciThroughput = CgrphSciThroughput
    ProductSciCoverage = CgrphSciCoverage
    ProductSciSnr = CgrphSciSnr
    ProductPsfMedian = CgrphPsfMedian

    def process(self) -> set[DataItem]:
        raw_images = cpl.core.ImageList()
        image = self._create_dummy_image()
        table = self._create_dummy_table()

        product_bkg = self.ProductBkg(self.header, image)
        product_bkg_subtracted = self.ProductBkgSubtracted(self.header, image)
        product_object_cat = self.ProductObjectCatalog(self.header, table)

        return {product_bkg, product_bkg_subtracted, product_object_cat}


class MetisImgAdiCgrph(MetisRecipe):
    _name = "metis_img_adi_cgrph"
    _version = "0.1"
    _author = "Martin Baláž, A*"
    _email = "martin.balaz@univie.ac.at"
    _copyright = "GPL-3.0-or-later"
    _synopsis = "Classical ADI postprocessing for CVC / RAVC coronagraphs"
    _description = ""

    parameters = ParameterList([
        ParameterEnum(
            name=f"{_name}.combination.method",
            context=_name,
            description="Name of the method used to combine the input images",
            default="add",
            alternatives=("add", "average", "median"),
        ),
        # Parameters for combination method
        # Resampling method
        # Parameters for resampling method
        # High-pass filtering method (Laplacian / median box)
        # Parameters for high-pass filtering (kernel / box size)
        ParameterRange(
            name=f"{_name}.contrast",
            min=0, # in lambda / D  ToDo change to sensible value
            max=1, # in lambda / D  ToDo change to sensible value
            default=0,
        ),
    ])

    _matched_keywords = {'DRS.FILTER'}
    _algorithm = """Average all or SKY exposures with object rejection
    Subtract background"""

    Impl = MetisImgAdiCgrphImpl
