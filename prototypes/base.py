from abc import ABCMeta, abstractmethod
from typing import Dict, Any

import cpl
from cpl.core import Msg


class MetisRecipe(cpl.ui.PyRecipe, metaclass=ABCMeta):
    """
        Common METIS-specific steps.
        We might want to refactor this some more, probably some more specific subclasses will show up.
    """
    _name: str = "undefined"
    _author: str = "undefined"
    _email: str = "undefined@undefined.undefined"
    _copyright: str = "GPL-3.0-or-later"
    _description: str = "Base class for METIS pipeline recipes"
    _synopsis: str = "Base class"
    _version: str = "1.0"

    parameters = cpl.ui.ParameterList([])

    def __init__(self) -> None:
        super().__init__()
        self.raw_frames = cpl.ui.FrameSet()
        self.frameset = None
        self.header = None
        self.raw_images = cpl.core.ImageList()
        self.raw_frames = cpl.ui.FrameSet()
        self.product_frames = cpl.ui.FrameSet()
        self.product_properties = cpl.core.PropertyList()
        self.output_file = self.get_output_file_name()

    def run(self, frameset: cpl.ui.FrameSet, settings: Dict[str, Any]) -> cpl.ui.FrameSet:
        """ Main function of the recipe """
        self.frameset = frameset

        self.import_settings(settings)      # Import and process the provided settings dict
        self.load_input(frameset)           # Load the input raw frames
        self.filter_raw_frames()            # Filter raw images based on keywords
        self.process_images()               # Do the actual processing
        self.add_product_properties()       # Add properties to the output product
        self.save_product()                 # Save the output product

        return self.product_frames

    def import_settings(self, settings: Dict[str, Any]) -> None:
        """ Update the recipe parameters with the values requested by the user """
        for key, value in settings.items():
            try:
                self.parameters[key].value = value
            except KeyError:
                Msg.warning(
                    self.name,
                    f"Settings includes '{key}':{value} but {self} has no parameter named {key}.",
                )

    def filter_raw_frames(self):
        """ Filter raw frames from the SOF """
        for idx, frame in enumerate(self.raw_frames):
            Msg.info(self.name, f"Processing #{idx}: {frame.file!r}...")

            if idx == 0:
                self.header = cpl.core.PropertyList.load(frame.file, 0)

            Msg.debug(self.name, "Loading image.")
            raw_image = cpl.core.Image.load(frame.file, extension=1)

            # Insert the processed image in an image list. Of course
            # there is also an append() method available.
            self.raw_images.insert(idx, raw_image)

    @abstractmethod
    def load_input(self, frameset) -> cpl.ui.FrameSet:
        return cpl.ui.FrameSet()

    @abstractmethod
    def process_images(self) -> cpl.ui.FrameSet:
        # Should be @abstractmethod
        return cpl.ui.FrameSet()

    @abstractmethod
    def add_product_properties(self):
        pass

    @abstractmethod
    def save_product(self) -> cpl.ui.FrameSet:
        pass

    @abstractmethod
    def get_output_file_name(self) -> str:
        return ""