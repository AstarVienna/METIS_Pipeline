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
from typing import Dict

import cpl
from cpl.core import Msg

from pymetis.base import MetisRecipeImpl
from pymetis.base.recipe import MetisRecipe
from pymetis.base.product import PipelineProduct
from pymetis.inputs import SinglePipelineInput, PipelineInputSet


class PersistenceInput(SinglePipelineInput):
    _title: str = "primary raw input"
    _tags: re.Pattern = re.compile(r"RAW")
    _group: cpl.ui.Frame.FrameGroup = cpl.ui.Frame.FrameGroup.RAW


class MetisDetPersistenceImpl(MetisRecipeImpl):

    class InputSet(PipelineInputSet):
        """Input to metis_det_persistence

        For now just a single raw image.
        """

        def __init__(self, frameset: cpl.ui.FrameSet):
            super().__init__(frameset)
            self.raw = PersistenceInput(frameset)
            self.inputs |= {self.raw}

    class Product(PipelineProduct):
        """A Persistence Map."""
        group = cpl.ui.Frame.FrameGroup.PRODUCT
        level = cpl.ui.Frame.FrameLevel.FINAL
        frame_type = cpl.ui.Frame.FrameType.IMAGE

        @property
        def category(self) -> str:
            return r"PERSISTENCE_MAP"

        @property
        def output_file_name(self) -> str:
            return fr"{self.category}.fits"

        @property
        def tag(self) -> str:
            return self.category

    inputset: InputSet = None

    def process_images(self) -> Dict[str, PipelineProduct]:
        """
        This is where the magic happens: all business logic of the recipe should be contained within this function.
        You can define extra private functions, or use functions from the parent classes:
        for instance combine_images is a helper function that takes a frameset and a method and returns
        a single combined frame that is used throughout the pipeline.
        """

        Msg.info(self.__class__.__qualname__, f"Starting processing image attribute.")

        raw = cpl.core.Image.load(self.inputset.raw.frame.file, extension=1)

        header = cpl.core.PropertyList.load(self.inputset.raw.frame.file, 0)

        self.products = {
            "PERSISTENCE_MAP": self.Product(self, header, raw),
        }

        return self.products


class MetisDetPersistence(MetisRecipe):
    """metis_det_persistence

    metis_det_persistence is usually run by ESO, since it needs all previous
    observations of the preceding X hours. However, it can also be run by
    individuals.
    """
    # Fill in recipe information
    _name = "metis_det_persistence"
    _version = "0.1"
    _author = "A*"
    _email = "hugo@buddelmeijer.nl"
    _copyright = "GPL-3.0-or-later"
    _synopsis = "Calculates persistence for a single frame"
    _description = """Calculate persistence."""

    parameters = cpl.ui.ParameterList([
        cpl.ui.ParameterEnum(
            name="metis_det_persistence.dummy",
            context="metis_det_persistence",
            description="dummy parameter to prevent pyesorex problems",
            default="1",
            alternatives=("0", "1"),
        )
    ])
    implementation_class = MetisDetPersistenceImpl
