from abc import ABCMeta, abstractmethod

import cpl
from cpl.core import Msg

PIPELINE = r"METIS"


class PipelineProduct(metaclass=ABCMeta):
    """
        The abstract base class for a pipeline product:
        one file with associated headers and a frame
    """
    def __init__(self,
                 recipe: 'Recipe',
                 header: cpl.core.PropertyList,
                 frame: cpl.ui.Frame,
                 *,
                 file_name: str):
        self.recipe = recipe
        self.header = header
        self.frame = frame
        self.properties = cpl.core.PropertyList()
        self.file_name = file_name

        self.add_properties()

    def add_properties(self):
        """ Hook for adding properties: by default it does not do anything """
        # Every product must have a ESO PRO CATG
        self.properties.append(
            cpl.core.Property(
                "ESO PRO CATG",
                cpl.core.Type.STRING,
                self.category,
            )
        )

    @property
    @abstractmethod
    def category(self) -> str:
        """ Every product must define ESO PRO CATG """
        pass

    def save(self):
        Msg.info(self.recipe.name, f"Saving product file as {self.file_name!r}.")
        cpl.dfs.save_image(
            self.recipe.frameset,
            self.recipe.parameters,
            self.recipe.frameset,
            self.frame,
            self.recipe.name,
            self.properties,
            PIPELINE,
            self.file_name,
            header=self.header,
        )

    @property
    @abstractmethod
    def output_file_name(self) -> str:
        """ Form the output file name (the detector part is variable) """
        return None
