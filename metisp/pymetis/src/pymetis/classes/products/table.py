import cpl

from pymetis.classes.products import PipelineProduct
from pymetis.classes.products.product import PIPELINE


class PipelineTableProduct(PipelineProduct):
    frame_type: cpl.ui.Frame.FrameType = cpl.ui.Frame.FrameType.TABLE

    def __init__(self,
                 recipe_impl: 'MetisRecipeImpl',
                 header: cpl.core.PropertyList,
                 table: cpl.core.Table):
        super().__init__(recipe_impl, header)
        self.table: cpl.core.Table = table

    def save_files(self):
        cpl.dfs.save_table(
            self.recipe.frameset,       # All frames for the recipe
            self.recipe.parameters,     # The list of input parameters
            self.recipe.used_frames,    # The list of frames actually used  FixMe currently not working as intended
            self.table,                 # Table to be saved
            self.recipe.name,           # Name of the recipe
            self.properties,            # Properties to be appended
            PIPELINE,
            self.output_file_name,
            header=self.header,
        )
