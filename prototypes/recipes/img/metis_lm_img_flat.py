import cpl

from prototypes.base import MetisRecipe
from prototypes.flat import MetisBaseImgFlatImpl
from prototypes.inputs import PipelineInputSet
from prototypes.inputs.raw import raw_input, master_dark_input


class MetisLmImgFlatImpl(MetisBaseImgFlatImpl):
    class InputSet(PipelineInputSet):
        def __init__(self, frameset: cpl.ui.FrameSet = None, **kwargs):
            self.raw = raw_input(tags=["LM_FLAT_LAMP_RAW"])(frameset)
            self.master_dark = master_dark_input(tags=["MASTER_DARK_{det}"], det="2RG")(frameset)
            self.inputs = [self.raw, self.master_dark]
            super().__init__(frameset, **kwargs)

    class Product(MetisBaseImgFlatImpl.Product):
        band: str = "LM"

    @property
    def detector_name(self) -> str:
        return "2RG"


class MetisLmImgFlat(MetisRecipe):
    # Fill in recipe information
    _name = "metis_lm_img_flat"
    _version = "0.1"
    _author = "Kieran Chi-Hung Hugo Gilles Martin"
    _email = "hugo@buddelmeijer.nl"
    _copyright = "GPL-3.0-or-later"
    _synopsis = "Create master flat for L/M band detectors"
    _description = "Prototype to create a METIS Masterflat."

    parameters = cpl.ui.ParameterList([
        cpl.ui.ParameterEnum(
            name=f"{_name}.stacking.method",
            context=_name,
            description="Name of the method used to combine the input images",
            default="average",
            alternatives=("add", "average", "median"),
        ),
    ])
    implementation_class = MetisLmImgFlatImpl
