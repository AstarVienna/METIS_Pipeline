import cpl
from cpl.core import Msg

from pymetis.classes.products import PipelineProduct
from pymetis.classes.products.product import PIPELINE


class PipelineMultipleProduct(PipelineProduct):
    """
    PipelineMultiProduct represents products with multi-extensions FITS files.
    The base structure is the PropertyList, with tables or images saved in extensions.
    """
    frame_type: cpl.ui.Frame.FrameType = cpl.ui.Frame.FrameType.IMAGE

    def __init__(self,
                 recipe_impl: 'MetisRecipeImpl',
                 header: cpl.core.PropertyList,
                 **extensions):
        super().__init__(recipe_impl, header)

        self.extensions = extensions
        for key, ext in self.extensions.items():
            self.__setattr__(key, ext)

    def save_files(self):
        cpl.dfs.save_propertylist(
            self.recipe.frameset,
            self.recipe.parameters,
            self.recipe.used_frames,
            self.recipe.name,
            self.properties,
            PIPELINE,
            self.output_file_name,
            header=self.header,
        )

        for key, ext in self.extensions.items():
            ext.save(self.output_file_name, cpl.core.PropertyList(), cpl.core.io.EXTEND)
