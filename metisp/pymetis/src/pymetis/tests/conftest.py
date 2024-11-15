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
import os

from pathlib import Path
from typing import Type

import cpl
from pyesorex.pyesorex import Pyesorex
from pymetis.base import MetisRecipe


root = Path(os.path.expandvars("$SOF_DIR"))


@pytest.fixture
def create_pyesorex():
    def inner(cls: Type[MetisRecipe]):
        p = Pyesorex()
        p.recipe = cls._name
        return p

    return inner


@pytest.fixture
def load_frameset():
    def inner(filename: str):
        frameset = cpl.ui.FrameSet()
        with open(root / filename) as f:
            for line in f.readlines():
                tokens = line.rstrip('\n').split(' ')
                path = os.path.expandvars(tokens[0])
                frameset.append(cpl.ui.Frame(path, tag=tokens[1]))

        return frameset
    return inner
