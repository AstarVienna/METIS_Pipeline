import pytest

from pymetis.inputs import PipelineInput


class TestPipelineInput:
    def test_fails_if_not_title_defined(self):
        class NoTitleInput(PipelineInput):
            pass

        with pytest.raises(TypeError):
            NoTitleInput()