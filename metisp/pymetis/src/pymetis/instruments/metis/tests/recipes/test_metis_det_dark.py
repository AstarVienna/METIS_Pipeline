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

import pytest

import cpl

from pymetis.instruments.metis.recipes.metis_det_dark import (MetisDetDark as Recipe,
                                                              MetisDetDarkImpl as Impl)
from tests.classes import BandParamRecipeTest, RawInputSetTest
from tests.classes.product import ImageProductSetTest

recipe_name = r'metis_det_dark'


@pytest.fixture
def name() -> str:
    return recipe_name


@pytest.fixture
def sof(name: str) -> str:
    return rf'{name}.ifu.sof'


class TestRecipe(BandParamRecipeTest):
    """ A bunch of simple and stupid test cases... just to see if it does something """
    Recipe = Recipe

    @staticmethod
    def _frameset(directory, *tags: str) -> cpl.ui.FrameSet:
        """ One (empty) FITS file per tag; classification and validation never open them. """
        frames = cpl.ui.FrameSet()
        for i, tag in enumerate(tags):
            filename = str(directory / f"{tag.lower()}_{i}.fits")
            cpl.core.PropertyList().save(filename, cpl.core.io.CREATE)
            frames.append(cpl.ui.Frame(filename, tag=tag))
        return frames

    def test_fails_with_raw_files_from_multiple_detectors(self, tmp_path):
        """ 2RG and GEO darks in one set of frames can never feed one input. """
        frames = self._frameset(tmp_path, 'DARK_2RG_RAW', 'DARK_GEO_RAW', 'GAIN_MAP_2RG', 'LINEARITY_2RG')
        with pytest.raises(cpl.core.IllegalInputError, match='DARK_2RG_RAW.*DARK_GEO_RAW'):
            Impl.InputSet(frames)

    def test_fails_with_calibrations_from_another_detector(self, tmp_path):
        """ 2RG darks with a GEO gain map used to validate and tag the master dark GEO. """
        frames = self._frameset(tmp_path, 'DARK_2RG_RAW', 'DARK_2RG_RAW', 'GAIN_MAP_GEO', 'LINEARITY_2RG')
        inputset = Impl.InputSet(frames)
        with pytest.raises(cpl.core.IllegalInputError, match="detector: raw has '2RG', gain_map has 'GEO'"):
            inputset.validate()


class TestInputSet(RawInputSetTest):
    Impl = Impl


class TestProduct(ImageProductSetTest):
    Product = Impl.ProductSet.MasterDark


