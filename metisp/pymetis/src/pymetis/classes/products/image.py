from __future__ import annotations

import cpl

from pymetis.classes.products import PipelineProduct
from pymetis.classes.products.product import PIPELINE


class PipelineImageProduct(PipelineProduct):
    frame_type: cpl.ui.Frame.FrameType = cpl.ui.Frame.FrameType.IMAGE

    def __init__(self,
                 recipe_impl: 'MetisRecipeImpl',
                 header: cpl.core.PropertyList,
                 image: cpl.core.Image,
                 *,
                 output_file_name: str = None):
        super().__init__(recipe_impl, header)
        self.image: cpl.core.Image = image

    def save_files(self, parameters: cpl.ui.ParameterList) -> None:
        cpl.dfs.save_image(
            self.recipe.frameset,       # All frames for the recipe
            parameters,                 # The list of input parameters
            self.recipe.used_frames,    # The list of frames actually used  FixMe currently not working as intended
            self.image,                 # Image to be saved
            self.recipe.name,           # Name of the recipe
            self.properties,            # Properties to be appended
            PIPELINE,
            self.output_file_name,
            header=self.header,
        )
