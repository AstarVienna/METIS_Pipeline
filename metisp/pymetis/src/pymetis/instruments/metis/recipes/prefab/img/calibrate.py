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

from abc import ABC

from pymetis.engine.dataitems import DataItem, Hdu, PipelineProductSet
from pymetis.engine.qc import QcParameter, QcParameterSet
from pymetis.engine.inputs import SinglePipelineInput, PipelineInputSet
from pymetis.engine.core.dummy import create_dummy_header

from pymetis.instruments.metis.mixins import TargetSciMixin
from pymetis.instruments.metis.dataitems.background.subtracted import BackgroundSubtracted
from pymetis.instruments.metis.dataitems.distortion.table import DistortionTable
from pymetis.instruments.metis.dataitems.img.basicreduced import Calibrated
from pymetis.instruments.metis.inputs import FluxCalTableInput
from pymetis.instruments.metis.recipes.base import MetisRecipeImpl


class MetisImgCalibrateImpl(TargetSciMixin, MetisRecipeImpl, ABC):
    class InputSet(PipelineInputSet):
        class BackgroundInput(SinglePipelineInput):
            Item = BackgroundSubtracted

        class FluxcalTableInput(FluxCalTableInput):
            pass

        # ToDo let's make TAB / TABLE consistent one day
        class DistortionTableInput(SinglePipelineInput):
            Item = DistortionTable

    class ProductSet(PipelineProductSet):
        SciCalibrated = Calibrated

    class Qc(QcParameterSet):
        """ No QC outputs """

    def process(self) -> set[DataItem]:
        background = self.inputset.background.load_data('DET1.DATA')
        primary_header = self.inputset.background.item.primary_header
        header = create_dummy_header()

        product_calibrated = self.ProductSet.SciCalibrated(
            primary_header,
            Hdu(header, background, name='DET1.DATA'),
        )

        return {product_calibrated}
