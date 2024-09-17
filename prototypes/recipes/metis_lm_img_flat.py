import cpl

from prototypes.base import MetisRecipe
from prototypes.flat import MetisBaseImgFlatImpl
from prototypes.mixins import MasterDarkInputMixin


class MetisLmImgFlatImpl(MetisBaseImgFlatImpl):
    class Input(MetisBaseImgFlatImpl.Input):
        tags_raw = ["LM_FLAT_LAMP_RAW"]
        tags_dark = ["MASTER_DARK_2RG", "MASTER_DARK_GEO", "MASTER_DARK_IFU"]
        # TODO This is probably not consistent with detector name

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
