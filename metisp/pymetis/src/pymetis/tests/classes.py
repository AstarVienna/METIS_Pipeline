import pytest

from pymetis.inputs import PipelineInput


class TestPipelineInput:
    def test_fails_if_no_title_defined(self):
        class NoTitleInput(PipelineInput):
            pass

        with pytest.raises(TypeError):
            NoTitleInput()

    def test_fails_if_no_group_defined(self):
        class NoGroupInput(PipelineInput):
            pass

        with pytest.raises(TypeError):
            NoGroupInput()