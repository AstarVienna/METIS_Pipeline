import os
import pytest

import cpl

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

