import os
import pytest
from typing import Type

from pyesorex.pyesorex import Pyesorex

from prototypes.base import MetisRecipeImpl

import cpl


@pytest.fixture
def create_pyesorex():
    def inner(cls: Type[MetisRecipeImpl]):
        p = Pyesorex()
        p.recipe = cls._name
        return p

    return inner


@pytest.fixture
def sof():
    def inner(filename: str):
        frameset = cpl.ui.FrameSet()
        with open(filename) as f:
            for line in f.readlines():
                tokens = line.rstrip('\n').split(' ')
                frameset.append(cpl.ui.Frame(os.path.expandvars(tokens[0]), tag=tokens[1]))

        return frameset
    return inner

