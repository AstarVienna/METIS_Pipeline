from abc import ABC

import inspect
import os
import pytest
from typing import Type

from pyesorex.pyesorex import Pyesorex

from pymetis.inputs.inputset import PipelineInputSet
from pymetis.base import MetisRecipe

import cpl


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
        with open(filename) as f:
            for line in f.readlines():
                tokens = line.rstrip('\n').split(' ')
                frameset.append(cpl.ui.Frame(os.path.expandvars(tokens[0]), tag=tokens[1]))

        return frameset
    return inner
