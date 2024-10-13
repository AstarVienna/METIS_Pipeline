import cpl
from cpl.core import Msg
from typing import Any, Dict, Literal

from pymetis.base import MetisRecipe, MetisRecipeImpl
from pymetis.input import PipelineInput
from pymetis.product import PipelineProduct


class MetisIfuCalibrateImpl(MetisRecipeImpl):
    @property
    def detector_name(self) -> str | None:
        return "2RG"

    class Input(PipelineInput):
        tag_sci_reduced = "IFU_SCI_REDUCED"
        tag_telluric = "IFU_TELLURIC"
        tag_fluxcal = "FLUXCAL_TAB"
        detector_name = '2RG'

        def __init__(self, frameset: cpl.ui.FrameSet):
            self.sci_reduced: cpl.ui.Frame | None = None
            self.telluric: cpl.ui.Frame | None = None
            self.fluxcal: cpl.ui.Frame | None = None
            super().__init__(frameset)

        def categorize_frame(self, frame: cpl.ui.Frame) -> None:
            match frame.tag:
                case self.tag_sci_reduced:
                    frame.group = cpl.ui.Frame.FrameGroup.RAW # TODO What group is this really?
                    self.sci_reduced = self._override_with_warning(self.sci_reduced, frame,
                                                                   origin=self.__class__.__qualname__,
                                                                   title="sci reduced")
                    Msg.debug(self.__class__.__qualname__, f"Got sci reduced frame: {frame.file}.")
                case self.tag_telluric:
                    frame.group = cpl.ui.Frame.FrameGroup.CALIB
                    self.telluric = self._override_with_warning(self.sci_reduced, frame,
                                                                origin=self.__class__.__qualname__,
                                                                title="sci reduced")
                    Msg.debug(self.__class__.__qualname__, f"Got telluric correction frame: {frame.file}.")
                case self.tag_fluxcal:
                    frame.group = cpl.ui.Frame.FrameGroup.CALIB
                    self.fluxcal = self._override_with_warning(self.fluxcal, frame,
                                                               origin=self.__class__.__qualname__,
                                                               title="flux calibration")
                    Msg.debug(self.__class__.__qualname__, f"Got a flux calibration frame: {frame.file}.")
                case _:
                    super().categorize_frame(frame)

        def verify(self) -> None:
            pass

    class ProductSciCubeCalibrated(PipelineProduct):
        category = rf"IFU_SCI_CUBE_CALIBRATED"

    def process_images(self) -> Dict[str, PipelineProduct]:
        # self.correct_telluric()
        # self.apply_fluxcal()

        self.products = {
            product.category: product()
            for product in [self.ProductSciCubeCalibrated]
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
    implementation_class = MetisIfuCalibrateImpl
