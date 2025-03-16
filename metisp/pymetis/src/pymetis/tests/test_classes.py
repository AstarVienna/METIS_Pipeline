import cpl
import pytest

from pymetis.classes.inputs.single import SinglePipelineInput


class TestPipelineInput:
    def test_fails_if_no_title_defined(self):
        class NoTitleInput(SinglePipelineInput):
            """
            Incorrect testing Input class with no title defined.
            """
            pass

        with pytest.raises(NotImplementedError):
            NoTitleInput(cpl.ui.FrameSet())

    def test_fails_if_no_group_defined(self):
        class NoGroupInput(SinglePipelineInput):
            """
            Incorrect testing Input class with no group defined.
            """

        with pytest.raises(NotImplementedError):
            NoGroupInput(cpl.ui.FrameSet())