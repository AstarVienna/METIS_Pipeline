import cpl
import pytest

from pymetis.inputs.single import SinglePipelineInput


class TestPipelineInput:
    def test_fails_if_no_title_defined(self):
        class NoTitleInput(SinglePipelineInput):
            pass

        with pytest.raises(NotImplementedError):
            NoTitleInput(cpl.ui.FrameSet())

    def test_fails_if_no_group_defined(self):
        class NoGroupInput(SinglePipelineInput):
            pass

        with pytest.raises(NotImplementedError):
            NoGroupInput(cpl.ui.FrameSet())