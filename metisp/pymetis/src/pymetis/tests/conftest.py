import pytest
import os

from pathlib import Path
from typing import Type

import cpl
from pyesorex.pyesorex import Pyesorex
from pymetis.base import MetisRecipe


root = Path(os.path.expandvars("$SOF"))


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
