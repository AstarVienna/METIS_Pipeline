import cpl
from cpl.core import Msg

from .base import MultiplePipelineInput, SinglePipelineInput


class InputFactory:
    def create(self,
               name: str,
               parents: [type],
               *,
               tags: [str] = None,
               **kwargs):
        tags = [tag.format(**kwargs) for tag in tags]    # Build all accepted tag names from tag prototypes and **kwargs

        # Finally construct and return the new class
        return type(
            name,
            parents,
            {
            }
        )



def raw_input(*, tags: [str] = None, detector: str = None, **kwargs) -> type:
    class RawInput(MultiplePipelineInput):
        _title: str = "raw"
        _group = cpl.ui.Frame.FrameGroup.RAW
        _tags: str = [tag.format(**kwargs) for tag in tags]

    return RawInput


def master_dark_input(*, tags: [str] = None, detector: str = None, **kwargs) -> type:
    class MasterDarkInput(SinglePipelineInput):
        _title: str = "master dark"
        _tags: str = [tag.format(**kwargs) for tag in tags]

    return MasterDarkInput


def linearity_input(tags: [str] = None) -> type:
    class LinearityInput(SinglePipelineInput):
        _title: str = "linearity map"
        _tags: str = tags

    return LinearityInput
