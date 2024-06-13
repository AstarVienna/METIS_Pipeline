from abc import ABCMeta, abstractmethod
from typing import Dict, Any

from cpl.core import Msg, PropertyList
from cpl.ui import PyRecipe, FrameSet


class MetisRecipe(PyRecipe, metaclass=ABCMeta):
    """ Common METIS-specific steps"""
    _name = "undefined"
    _author = "undefined"
    _email = "undefined@undefined.undefined"
    _copyright = "GPL-3.0-or-later"
    _description = "Base class for METIS pipeline recipes"
    _synopsis = "Base class"
    _version = "1.0"

    def __init__(self) -> None:
        self.raw_frames = FrameSet()
        self.frameset = None
        self.header = None
        self.product_frames = FrameSet()
        self.product_properties = PropertyList()

    def run(self, frameset: FrameSet, settings: Dict[str, Any]) -> FrameSet:
        """ Main function of the recipe """
        self.frameset = frameset

        self.process_settings(settings)     # Process the provided settings dict
        self.load_input(frameset)           # Load the input raw frames
        self.do_everything()
        self.produce_output()
        return self.product_frames

    def process_settings(self, settings: Dict[str, Any]) -> None:
        """ Update the recipe parameters with the values requested by the user """
        for key, value in settings.items():
            try:
                self.parameters[key].value = value
            except KeyError:
                Msg.warning(
                    self.name,
                    f"Settings includes '{key}':{value} but {self} has no parameter named {key}.",
                )

    def load_input(self) -> FrameSet:
        return FrameSet()

    def produce_output(self) -> FrameSet:
        return FrameSet()
