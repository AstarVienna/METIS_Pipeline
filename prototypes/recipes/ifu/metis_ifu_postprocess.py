import cpl
from cpl.core import Msg
from typing import Dict

from prototypes.base.impl import MetisRecipe, MetisRecipeImpl
from prototypes.base.input import RecipeInput
from prototypes.base.product import PipelineProduct
from prototypes.recipes.ifu.metis_ifu_distortion import MetisIfuDistortionImpl


class MetisIfuPostprocessImpl(MetisRecipeImpl):
    @property
    def detector_name(self) -> str | None:
        return "2RG"

    class Input(RecipeInput):
        tag_sci_cube_calibrated = "IFU_SCI_CUBE_CALIBRATED"
        detector_name = '2RG'

        def __init__(self, frameset: cpl.ui.FrameSet):
            self.sci_cube_calibrated: cpl.ui.Frame | None = None
            super().__init__(frameset)

        def categorize_frame(self, frame: cpl.ui.Frame) -> None:
            match frame.tag:
                case self.tag_sci_cube_calibrated:
                    frame.group = cpl.ui.Frame.FrameGroup.RAW # TODO What group is this really?
                    self.sci_cube_calibrated = self._override_with_warning(self.sci_cube_calibrated, frame,
                                                                           origin=self.__class__.__qualname__,
                                                                           title="sci cube calibrated")
                    Msg.debug(self.__class__.__qualname__, f"Got sci cube calibrated frame: {frame.file}.")
                case _:
                    super().categorize_frame(frame)

        def verify(self) -> None:
            pass

    class ProductSciCoadd(PipelineProduct):
        category = rf"IFU_SCI_COADD"

    def process_images(self) -> Dict[str, PipelineProduct]:
        # self.determine_output_grid()
        # self.resample_cubes()
        # self.coadd_cubes()

        header = cpl.core.PropertyList.load(self.input.sci_cube_calibrated.file, 0)
        coadded_image = cpl.core.Image()

        self.products = {
            'IFU_SCI_COADD': self.ProductSciCoadd(self, header, coadded_image, detector_name=self.detector_name),
        }
        return self.products


class MetisIfuCalibrate(MetisRecipe):
    _name = "metis_ifu_calibrate"
    _version = "0.1"
    _author = "Martin Baláž"
    _email = "martin.balaz@univie.ac.at"
    _copyright = "GPL-3.0-or-later"
    _synopsis = "Calibrate IFU science data"
    _description = (
        "Currently just a skeleton prototype."
    )

    parameters = cpl.ui.ParameterList([])
    implementation_class = MetisIfuDistortionImpl
